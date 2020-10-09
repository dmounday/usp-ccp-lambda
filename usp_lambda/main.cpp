/*
 * main.cpp
 *
 *  Created on: Mar 17, 2020
 *      Author: dmounday
 */
#include <aws/core/Aws.h>
#include <aws/core/utils/memory/stl/AWSAllocator.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
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
#include <aws/iot-data/model/GetThingShadowRequest.h>
#include <aws/iot-data/model/GetThingShadowResult.h>
#include <aws/states/SFNClient.h>
#include <aws/states/model/SendTaskSuccessRequest.h>
#include <aws/lambda-runtime/runtime.h>

#include <cstdlib>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/util/json_util.h>

#include "usp-msg-1-1.pb.h"
#include "usp-record-1-1.pb.h"
#include "USPMessage.h"
#include "USPresponse.h"
#include "NotifyResponse.h"

#define USP_HANDLER "usp_decode"

 const char* end_point;          // AWS MQTT Borker's EndPoint
 const std::string ReplyTo{"/reply-to="};

 std::string ParseReplyTo(const std::string& topic)
 {
   std::size_t p = topic.find(ReplyTo);
   if ( p != std::string::npos ){
     std::string respond = topic.substr(p + ReplyTo.length());
     // replace %2F with '/'
     std::size_t s{0};
     while ( (s = respond.find("%2F", s))!= std::string::npos){
       respond.replace(s, 3, "/");
       s += 3;
     }
     return respond;
   }
   AWS_LOGSTREAM_ERROR(USP_HANDLER, "MQTT Topic does not contain reply-to ");
   return std::string{};
 }
 /*
  * SendTaskSuccess with TaskToken and USP response as Json.
  */
void SendSuccess(Aws::String& token, std::string json_msg){
	AWS_LOGSTREAM_DEBUG(USP_HANDLER, "SendSuccess: "<< token);
	AWS_LOGSTREAM_INFO(USP_HANDLER, json_msg );
	Aws::Client::ClientConfiguration config;
	config.verifySSL = false;
	Aws::SFN::SFNClient client(config);
	Aws::SFN::Model::SendTaskSuccessRequest request;
	request.SetTaskToken(token);
	request.SetOutput(json_msg.c_str());
	Aws::SFN::Model::SendTaskSuccessOutcome outcome (client.SendTaskSuccess(request));
	if (outcome.IsSuccess()){
		AWS_LOGSTREAM_DEBUG(USP_HANDLER, "SendTaskSuccess");
	}else{
		AWS_LOGSTREAM_ERROR(USP_HANDLER, outcome.GetError().GetMessage().c_str());
	}
}

 Aws::String FindTaskToken( Aws::IOStream& ss ){
	 using namespace Aws::Utils::Json;
	 const JsonValue shadow_json(ss);
	 assert(shadow_json.WasParseSuccessful());
	 JsonView view = shadow_json;
	 JsonView state = view.GetObject("state");
	 JsonView desired = state.GetObject("desired");
	 Aws::String token = desired.GetString("task-token");
	 AWS_LOGSTREAM_DEBUG(USP_HANDLER, "task-token" <<token.c_str());
	 return token;
 }

 Aws::String GetTaskToken( Aws::String thing ){
	 Aws::IoTDataPlane::Model::GetThingShadowRequest request;
	 Aws::Client::ClientConfiguration config;
	 config.verifySSL = false;
	 Aws::IoTDataPlane::IoTDataPlaneClient client(config);

	 request.SetThingName(thing);
	 client.OverrideEndpoint(end_point);
	 Aws::IoTDataPlane::Model::GetThingShadowOutcome outcome(client.GetThingShadow(request));
	 if (outcome.IsSuccess()){
		 Aws::IoTDataPlane::Model::GetThingShadowResult& result = outcome.GetResult();
		 Aws::IOStream& ss = result.GetPayload();
		 return FindTaskToken(ss);
	 } else {
		 AWS_LOGSTREAM_ERROR(USP_HANDLER, outcome.GetError().GetMessage().c_str());
	 }
	 return Aws::String("");
 }

 void PublishResponse(const std::string &topic, std::shared_ptr<Aws::StringStream> sp) {
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
 USPEncodeRecord(const usp::Msg& usp_message,
                   const std::string& to_endpoint,
                   const std::string& from_endpoint,
				   std::string* response_string){
   usp_record::Record record;
   record.set_version("1.0");  // ToDo: move to consts file.
   record.set_to_id(to_endpoint);
   record.set_from_id(from_endpoint);
   record.set_payload_security(usp_record::Record::PLAINTEXT);
   usp_record::NoSessionContextRecord* nc =record.mutable_no_session_context();
   nc->set_payload(usp_message.SerializeAsString());
   record.SerializeToString( response_string );
 }

void SendNotifyResponse(const std::string &topic,
		const std::string &to_endpoint, const std::string &from_endpoint,
		const std::string &id, const usp::Notify &notify) {
	gsagent::NotifyResponse response(id, notify.subscription_id());
	usp::Msg usp_message = *response.get_message();
	std::string usp_msg_buf;
	USPEncodeRecord(usp_message, to_endpoint, from_endpoint, &usp_msg_buf);
	AWS_LOGSTREAM_INFO(USP_HANDLER, "received topic: " << topic);
	// The received topic should have the format of "topic/reply-to=<agent-topic>
	// such as: usp-control-aws/reply-to=gs_agent-02%2F     '/' is converted to %2F .
	std::string reply_topic = ParseReplyTo(topic);
	if (reply_topic.length()) {
		auto sp = Aws::MakeShared<Aws::StringStream>(USP_HANDLER); // USP_HANDLER is an allocation tag.
		sp->str(usp_msg_buf.c_str());  // Set contents of stream.
		PublishResponse(reply_topic, sp);
	}
}


using namespace aws::lambda_runtime;
/*
 * invocation_request payload contains Json structure:
 * with following:
 * data: base64 encoded usp record payload
 * context:
 * cognito_id:
 * accountit:
 * topic:
 * clientid:
 */
static invocation_response my_handler(invocation_request const &req) {
	using namespace Aws::Utils::Json;
	Aws::String error_msg { "No data tag in payload." };
	Aws::String aws_str(req.payload.c_str(), req.payload.length());

	const JsonValue eventJson(aws_str);
	assert(eventJson.WasParseSuccessful());
	JsonView data = eventJson;

	AWS_LOGSTREAM_INFO(USP_HANDLER, "client-id: " << data.GetString("clientid").c_str());
	if (data.ValueExists("data")) {
		Aws::String encoded_data = data.GetString("data");
		Aws::String cid = data.GetString("topic");
		std::string topic(cid.c_str(), cid.length());
		Aws::Utils::ByteBuffer usp_record =
				Aws::Utils::HashingUtils::Base64Decode(encoded_data);
		//hex_dump(std::cout, usp_record.GetUnderlyingData(),
		//		usp_record.GetLength(), true);
		usp_record::Record response_record;
		if (response_record.ParseFromArray(usp_record.GetUnderlyingData(),
				usp_record.GetLength())) {
			usp_record::NoSessionContextRecord nsc_ =
					response_record.no_session_context();
			std::string to_endpoint = response_record.to_id();
			std::string from_endpoint = response_record.from_id();
			usp::Msg usp_msg;
			if (usp_msg.ParseFromString(nsc_.payload())) {
				// consider using google.protobuf.json_format.MessageToJson here.
				AWS_LOGSTREAM_INFO(USP_HANDLER, usp_msg.DebugString());

				if (usp_msg.body().request().has_notify()) {
					const usp::Notify &notify =
							usp_msg.body().request().notify();
					if (notify.send_resp()) {
						SendNotifyResponse(topic,
								from_endpoint, to_endpoint,
								usp_msg.header().msg_id(), notify);
						error_msg = "OK";
					}
				}
				Aws::String token = GetTaskToken( data.GetString("clientid") );
				if (token.length() > 0 ){
					AWS_LOGSTREAM_DEBUG(USP_HANDLER, "********** TaskToken: " << token.c_str());
					// send to step function associated with token.
					namespace pbu = google::protobuf::util;
					std::string json_usp_msg;
					pbu::Status status = pbu::MessageToJsonString(usp_msg, &json_usp_msg);
					if ( status.ok()) {
						SendSuccess(token, json_usp_msg);
					}
				} else {
					AWS_LOGSTREAM_ERROR(USP_HANDLER, "thing missing task token");
					error_msg ="Thing missing Task Token";
				}

			} else {
				error_msg = "Usp Msg parse error";/* msg parse error */
			}
		} else {
			error_msg = "USP record parse error";/* protobuf parse error */
		}
	}
	AWS_LOGSTREAM_FLUSH();
	JsonValue response;
	response.WithString("body", error_msg).WithInteger("statusCode", 200);
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

    {
        aws::lambda_runtime::run_handler(my_handler);
    }
    ShutdownAPI(options);
    return 0;
}




