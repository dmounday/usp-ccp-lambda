/*
 * main.cpp
 *
 *  Created on: Mar 17, 2020
 *      Author: dmounday
 */
#include <aws/core/Aws.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/core/utils/logging/LogLevel.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/LogMacros.h>
#include <aws/core/utils/json/JsonSerializer.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/utils/Outcome.h>
#include <aws/core/utils/Array.h>
#include <aws/core/platform/Environment.h>
#include <aws/core/client/ClientConfiguration.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/AmazonStreamingWebServiceRequest.h>
#include <aws/iot-data/IoTDataPlaneClient.h>
#include <aws/iot-data/IoTDataPlaneRequest.h>
#include <aws/iot-data/model/PublishRequest.h>
#include <aws/iot-data/model/UpdateThingShadowRequest.h>
#include <aws/lambda-runtime/runtime.h>

#include <cstdlib>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/util/type_resolver.h>
#include <google/protobuf/util/type_resolver_util.h>
#include "usp-msg-1-1.pb.h"
#include "usp-record-1-1.pb.h"
#include "USPMessage.h"
#include "USPresponse.h"
#include "NotifyResponse.h"

#define USP_HANDLER "usp_encode"

 const char* end_point;          // AWS MQTT Borker's EndPoint
 const char* from_identifier;	 // Used as from-id in USP Record. Also the publish to topic for agent.
 const std::string ReplyTo{"/reply-to="}; // used to construct publish to topic

/*
 * SetReplyToTopic for USP Agent
 *      /reply-to=abcdef%2Fxyx%2Fdef
 * To be appended to topic.
 */
std::string SetReplyToTopic(const char* reply, std::size_t lth)
 {
	std::string reply_to;
   // replace '/' with "%2F"
   reply_to = ReplyTo;
   auto it = reply;
   for (std::size_t i=0; i!=lth; ++i, ++it ){
     if ( *it == '#' || *it == '+')
       continue;
     if ( *it != '/')
       reply_to += *it;
     else if ( i == lth-1)  // if / is last skip it.
       continue;
     else
       reply_to += "%2F";
   }
   return reply_to;
 }

void UpdateShadow(const Aws::String &thing, const Aws::String& token){
	auto sp = Aws::MakeShared<Aws::StringStream>(USP_HANDLER);
	*sp << R"({"state":{"desired":{"task-token": ")";
	*sp << token;
	*sp << R"("}}})";
	std::cout << "************* shadow update: " << sp->str();
	Aws::IoTDataPlane::Model::UpdateThingShadowRequest request;
	request.SetBody( sp );
	request.SetThingName( thing );
	Aws::Client::ClientConfiguration config;
	config.verifySSL = false;
	Aws::IoTDataPlane::IoTDataPlaneClient client(config);
	client.OverrideEndpoint(Aws::String(end_point));
	Aws::IoTDataPlane::Model::UpdateThingShadowOutcome outcome (client.UpdateThingShadow(request));
 	if (outcome.IsSuccess()) {
 		AWS_LOGSTREAM_DEBUG(USP_HANDLER, "UpdateThingShadow Success");
 	} else {
 		AWS_LOGSTREAM_ERROR(USP_HANDLER, outcome.GetError().GetMessage().c_str());
 	}

}

 void PublishRequest(const std::string &topic, std::shared_ptr<Aws::StringStream> sp) {
 	Aws::IoTDataPlane::Model::PublishRequest request;
	Aws::Client::ClientConfiguration config;
	config.verifySSL = false;
 	Aws::IoTDataPlane::IoTDataPlaneClient client(config);
 	client.OverrideEndpoint(Aws::String(end_point));

 	request.SetBody(sp );
 	request.SetTopic(topic.c_str());
 	request.SetContentType("application/octet-stream");

 	Aws::IoTDataPlane::Model::PublishOutcome outcome(client.Publish(request));
 	if (outcome.IsSuccess()) {
 		AWS_LOGSTREAM_DEBUG(USP_HANDLER, "PublishResponse Success");
 	} else {
 		AWS_LOGSTREAM_ERROR(USP_HANDLER, outcome.GetError().GetMessage().c_str());
 	}
 }

 void
 USPEncodeRecord(const std::string& msg_binary,
                   const Aws::String& to_endpoint,
                   const Aws::String& from_endpoint,
				   std::string* response_string){
   usp_record::Record record;
   record.set_version("1.0");  // ToDo: move to consts file.
   record.set_to_id(to_endpoint.c_str(), to_endpoint.length());
   record.set_from_id(from_endpoint.c_str(), from_endpoint.length());
   record.set_payload_security(usp_record::Record::PLAINTEXT);
   usp_record::NoSessionContextRecord* nc =record.mutable_no_session_context();
   nc->set_payload(msg_binary);
   record.SerializeToString( response_string );
 }

void from_json(google::protobuf::Message& msg,
                                     const char* first,
                                     std::size_t size,
									  std::string* binary_buffer)
{
    namespace pb = google::protobuf;
    namespace pbu = google::protobuf::util;
    std::cout << "size: " << size << std::endl;
    std::cout << "from_json:" << first << std::endl;
    auto resolver = pbu::NewTypeResolverForDescriptorPool("", pb::DescriptorPool::generated_pool());
    google::protobuf::StringPiece json_in(first, size);
    std::string type_url = "/"+msg.GetDescriptor()->full_name();

    auto status = pbu::JsonToBinaryString(resolver,
                                          type_url,
                                          json_in, binary_buffer);
   std::cout << "Status: "<< status.ToString() << std::endl;
}

using namespace aws::lambda_runtime;
/*
 * invocation request payload contains Json Structure:
 * Typical payload with USP Msg request:
 {
 "task-token": "2452345249529048",
 "to-id": "gs-agent-01",
 "from-id": "usp_controller_aws",
 "usp-msg": {
	 "header": {
	  "msgId": "101",
	  "msgType": "GET"
	 },
	 "body": {
	  "request": {
	   "get": {
	    "paramPaths": [
	     "Device.DeviceInfo.FriendlyName"
	    ]
	   }
	  }
	 }
 }
}
 *   client-id: Id of IoT device known as AWS IoT Thing
 *   from-id: This controller's End-point ID- This is also contents of reply-to=xxxx
 *   msg: USP message in Json form.
 */
static invocation_response my_handler(invocation_request const &req) {
	using namespace Aws::Utils::Json;

	Aws::String payload(req.payload.c_str(), req.payload.length());
	std::cout << req.payload << std::endl;

	const JsonValue eventJson(payload);
	assert(eventJson.WasParseSuccessful());
	JsonView view = eventJson;

	AWS_LOGSTREAM_INFO(USP_HANDLER, "client-id" << view.GetString("clientid").c_str());
	std::cout << "clientid: "<< view.GetString("to-id").c_str() << std::endl;
	Aws::String task_token = view.GetString("task-token");
	Aws::String to_id = view.GetString("to-id");
	Aws::String from_id = from_identifier;

    JsonView msg_view = view.GetObject("usp-msg");
    Aws::String usp_msg_json = msg_view.WriteReadable();
	usp::Msg msg;
	std::string binary_msg;
	binary_msg.reserve(usp_msg_json.length());
	std::string record_string;
	from_json( msg, usp_msg_json.c_str(), usp_msg_json.length(), &binary_msg);
	USPEncodeRecord( binary_msg, to_id, from_id, &record_string);
	// The publish to topic format is "topic/reply-to=<controller-topic>
	// such as: gs-agent-01/reply-to=usp-controller-01-aws%2F     '/' is converted to %2F .
	std::string to_id_str{to_id.c_str(), to_id.length()};
	std::string pub_topic = to_id_str + SetReplyToTopic(from_id.c_str(), from_id.length());
	auto sp = Aws::MakeShared<Aws::StringStream>(USP_HANDLER); // USP_HANDLER is an allocation tag.
	sp->str(record_string.c_str());  // Set contents of stream.
	PublishRequest(pub_topic, sp);
	// Update shadow with taks token from state-maching
	if ( task_token.length()> 0)
		UpdateShadow(to_id, task_token);
	else
		AWS_LOGSTREAM_ERROR(USP_HANDLER, "Missing task-token");

	AWS_LOGSTREAM_FLUSH();
	JsonValue response;
	response.WithString("body", "OK").WithInteger("statusCode", 200);
	auto const apig_response = response.View().WriteCompact();
	std::string resp(apig_response.c_str(), apig_response.length());
	return aws::lambda_runtime::invocation_response::success(resp,
			"application/json");

}
std::function<std::shared_ptr<Aws::Utils::Logging::LogSystemInterface>()> GetConsoleLoggerFactory()
{
    return [] {
        return Aws::MakeShared<Aws::Utils::Logging::ConsoleLogSystem>(
            "console_logger", Aws::Utils::Logging::LogLevel::Trace);
    };
}

int main()
{
    using namespace Aws;

    SDKOptions options;
    if ( const char* env_log = std::getenv("LogLevel")){
    	if ( !strcmp(env_log, "trace"))
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    	else if (!strcmp(env_log, "fatal"))
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Fatal;
    	else if (!strcmp(env_log,"warn"))
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Warn;
    	else if (!strcmp(env_log,"info"))
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Info;
    	else if (!strcmp(env_log, "Debug"))
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Debug;
    	else
    		options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Error;
    }
    options.loggingOptions.logger_create_fn = GetConsoleLoggerFactory();
    InitAPI(options);
   	if (const char* env = std::getenv("EndPoint")) {
   		end_point = env;
   	} else {
   		AWS_LOGSTREAM_FATAL("Configuration", "Must specify EndPoint environment variable");
   		return (-1);
   	}
   	if (const char* env = std::getenv("FromId")) {
   		from_identifier = env;
   	} else {
   		AWS_LOGSTREAM_FATAL("Configuration", "Must specify FromId environment variable");
   		return (-1);
   	}

    {
        aws::lambda_runtime::run_handler(my_handler);
    }
    ShutdownAPI(options);
    return 0;
}




