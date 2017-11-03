#pragma warning(disable:4996)
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include "HttpClient.h"
#include <curl.h>
#include <time.h>
#include <chrono>
#include <atomic>
#include <json.h>
#include <codecvt>
#include <locale>
#include "Utility.h"

using namespace std;
using namespace xnet;

using RequestPtr	= shared_ptr<HttpRequest>;
using ResponsePtr	= shared_ptr<HttpResponse>;

std::mutex mutex_;

void onMessage(const RequestPtr& request, const ResponsePtr& response)
{
	std::lock_guard<std::mutex> lock(mutex_);

	cout << "\n" << request->getTag() << endl;
	if (response->isSucceed())
	{
		cout << "HTTP request succeed!" << endl;

		auto utf8Str = response->responseDataAsString();
		
		Json::Value root;
		Json::Reader reader;
		reader.parse(utility::UTF8_To_GBK(utf8Str), root);
		cout << root << endl;
	}
	else
	{
		cout << "HTTP request failed!" << endl;
		cout << "status code: " << response->getResponseCode() << endl;
		cout << "reason: " << response->gerErrorBuffer() << endl;
	}

}

//GET HTTP failed
void testCase1(bool isImmediate = false)
{
	RequestPtr request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::GET);
	request->setUrl("http://just-make-this-request-failed.com");
	request->setCallback(onMessage);
	if (isImmediate)
	{
		request->setTag("test case 1: GET immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 1: GET");
		HttpClient::getInstance()->send(request);
	}
}

//GET HTTP
void testCase2(bool isImmediate = false)
{
	RequestPtr request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::GET);
	request->setUrl("http://httpbin.org/ip");
	request->setCallback(onMessage);
	if (isImmediate)
	{
		request->setTag("test case 3: GET immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 2: GET");
		HttpClient::getInstance()->send(request);
	}
}

//GET HTTPS
void testCase3(bool isImmediate = false)
{
	RequestPtr request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::GET);
	request->setUrl("https://httpbin.org/get");
	request->setCallback(onMessage);
	if (isImmediate)
	{
		request->setTag("test case 3: GET immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 3: GET");
		HttpClient::getInstance()->send(request);
	}
}

//POST
void testCase4(bool isImmediate = false)
{
	RequestPtr request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::POST);
	request->setUrl("http://httpbin.org/post");
	request->setCallback(onMessage);
	request->setRequestData("visitor=zhangkuo&time=20171103");
	if (isImmediate)
	{
		request->setTag("test case 4: POST immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 4: POST");
		HttpClient::getInstance()->send(request);
	}
}

//POST set Content-Type
void testCase5(bool isImmediate = false)
{
	auto request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::POST);
	request->setUrl("http://httpbin.org/post");
	request->setCallback(onMessage);

	std::vector<std::string> headers;
	headers.push_back("Content-Type: application/json; charset=utf-8");
	request->setHeaders(std::move(headers));

	Json::Value msg;
	msg["visitor"] = "zhangkuo";
	msg["time"] = 20171103;
	request->setRequestData(msg.toStyledString());

	if (isImmediate)
	{
		request->setTag("test case 5: POST immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 5: POST");
		HttpClient::getInstance()->send(request);
	}
}

//POST binary
void testCase6(bool isImmediate = false)
{
	auto request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::POST);
	request->setUrl("http://httpbin.org/post");
	request->setCallback(onMessage);
	const char* p = "binary data: I love cpp!\0\0from zhangkuo";
	request->setRequestData(p, 39);
	if (isImmediate)
	{
		request->setTag("test case 6: POST binary immediate");
		HttpClient::getInstance()->send(request);
	}
	else
	{
		request->setTag("test case 6: POST binary");
		HttpClient::getInstance()->sendImmediate(request);
	}
}

//PUT
void testCase7(bool isImmediate = false)
{
	auto request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::PUT);
	request->setUrl("http://httpbin.org/put");
	request->setCallback(onMessage);
	request->setRequestData("visitor=zhangkuo&time=20171103");
	request->setUploadFilePath("./curl/lib/libcurl.lib");
	if (isImmediate)
	{
		request->setTag("test case 7: PUT immediate");
		HttpClient::getInstance()->sendImmediate(request);
	}
	else
	{
		request->setTag("test case 7: PUT");
		HttpClient::getInstance()->send(request);
	}
}

//DELETE
void testCase8(bool isImmediate = false)
{
	auto request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::DELETE);
	request->setUrl("http://httpbin.org/delete");
	request->setCallback(onMessage);
	if (isImmediate)
	{
		request->setTag("test case 8: DELETE");
		HttpClient::getInstance()->send(request);
	}
	else
	{
		request->setTag("test case 8: DELETE immediate");
		HttpClient::getInstance()->send(request);
	}
}

void testAllCase()
{
	testCase1();
	testCase1(true);
	testCase2();
	testCase2(true);
	testCase3();
	testCase3(true);
	testCase4();
	testCase4(true);
	testCase5();
	testCase5(true);
	testCase6();
	testCase6(true);
	testCase7();
	testCase7(true);
	testCase8();
	testCase8(true);
}

void main()
{	
	auto request = make_shared<HttpRequest>();
	request->setRequestType(HttpRequest::Type::GET);
	request->setUrl("http://music.163.com/api/playlist/detail?id=387699584");
	request->setCallback(onMessage);
	HttpClient::getInstance()->send(request);

	while (std::getchar() == 'q' || std::getchar() == 'Q')
	{
		break;
	}
}