#pragma warning(disable:4996)
#include "HttpClient.h"
#include <memory>
#include <curl.h>
using namespace xnet;

static size_t writeCallback(void* data, size_t size, size_t nMember, void* stream)
{
	std::vector<char>* recvBuf = static_cast<std::vector<char>*>(stream);
	size_t writeSize = size * nMember;
	recvBuf->insert(recvBuf->end(), (char*)data, (char*)data + writeSize);

	return writeSize;
}

static size_t readCallback(void* data, size_t size, size_t nMember, void* stream)
{
	size_t nRead = fread(data, size, nMember, (FILE*)stream);
	return nRead;
}

class Curl
{
	using RequestPtr = std::shared_ptr<HttpRequest>;
	using ResponsePtr = std::shared_ptr<HttpResponse>;
public:
	Curl() :
		curl_(curl_easy_init()),
		headers_(nullptr)
	{
	}

	~Curl()
	{
		if (curl_) curl_easy_cleanup(curl_);

		if (headers_) curl_slist_free_all(headers_);
	}

	bool init(const HttpClient&	client,
		const RequestPtr& request,
		void* responseData,
		void* responseHeader,
		char* errorBuf)
	{
		if (curl_ == nullptr) return false;
		/*set no signal*/
		if (!setOption(CURLOPT_NOSIGNAL, 1L)) return false;

		/*set accept encoding*/
		if (!setOption(CURLOPT_ACCEPT_ENCODING, "")) return false;

		/*set cookie*/
		std::string cookieFilename = client.getCookieFilename();
		if (!cookieFilename.empty())
		{
			if (!setOption(CURLOPT_COOKIEFILE, cookieFilename.data()))
			{
				return false;
			}
			if (!setOption(CURLOPT_COOKIEJAR, cookieFilename.data()))
			{
				return false;
			}
		}

		/*set timeout*/
		if (!setOption(CURLOPT_TIMEOUT, client.getTimeoutForRead()))
		{
			return false;
		}
		/*set connect timeout*/
		if (!setOption(CURLOPT_CONNECTTIMEOUT, client.getTimeoutForConnect()))
		{
			return false;
		}
		/*set ssl*/
		std::string sslCaFilename = client.getSSLVerification();
		if (sslCaFilename.empty())
		{
			if (!setOption(CURLOPT_SSL_VERIFYPEER, false)) return false;
			if (!setOption(CURLOPT_SSL_VERIFYHOST, false)) return false;
		}
		else
		{
			if (!setOption(CURLOPT_SSL_VERIFYPEER, 1L)) return false;
			if (!setOption(CURLOPT_SSL_VERIFYHOST, 2L)) return false;
			if (!setOption(CURLOPT_CAINFO, sslCaFilename.data()))
			{
				return false;
			}
		}
		/*set header*/
		std::vector<std::string> headers = request->getHeaders();
		if (!headers.empty())
		{
			for (auto& header : headers)
			{
				headers_ = curl_slist_append(headers_, header.c_str());
			}

			if (!setOption(CURLOPT_HTTPHEADER, headers_))
			{
				return false;
			}
		}
		/*set main option*/
		return	setOption(CURLOPT_URL, request->getUrl()) &&
			setOption(CURLOPT_WRITEFUNCTION, writeCallback) &&
			setOption(CURLOPT_WRITEDATA, responseData) &&
			setOption(CURLOPT_HEADERFUNCTION, writeCallback) &&
			setOption(CURLOPT_HEADERDATA, responseHeader) &&
			setOption(CURLOPT_ERRORBUFFER, errorBuf);
	}

	template<typename T>
	bool setOption(CURLoption option, T data)
	{
		return CURLE_OK == curl_easy_setopt(curl_, option, data);
	}

	bool perform(long* responseCode)
	{
		if (CURLE_OK != curl_easy_perform(curl_)) return false;

		CURLcode code = curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, responseCode);
		if (code != CURLE_OK || !(*responseCode >= 200 && *responseCode < 300))
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(code));
			return false;
		}

		return true;
	}

private:
	CURL*		curl_;
	curl_slist* headers_;
};

HttpClient* HttpClient::instance_ = new HttpClient();

HttpClient::HttpClient() :
	timeoutForConnect_(30),
	timeoutForRead_(60),
	threadPool_("HttpThreadPool")
{
	threadPool_.start(kNumThreads);
	curl_global_init(CURL_GLOBAL_WIN32);
}

HttpClient::~HttpClient()
{
	curl_global_cleanup();
}

void HttpClient::enableCookies(const std::string& cookieFile)
{
	LockGuard lock(cookieFileMutex_);
	if (!cookieFile.empty())
	{
		cookieFilename_ = cookieFile;
	}
	else
	{
		cookieFilename_ = "cookieFile.txt";
	}
}

const std::string& HttpClient::getCookieFilename() const
{
	LockGuard lock(cookieFileMutex_);
	return cookieFilename_;
}

void HttpClient::setSSLVerification(const std::string& caFile)
{
	LockGuard lock(sslCaFileMutex_);
	sslCaFilename_ = caFile;
}

void HttpClient::setSSLVerification(std::string&& caFile)
{
	LockGuard lock(sslCaFileMutex_);
	sslCaFilename_ = std::move(caFile);
}

const std::string& HttpClient::getSSLVerification() const
{
	LockGuard lock(sslCaFileMutex_);
	return sslCaFilename_;
}

void HttpClient::setTimeoutForConnect(int value)
{
	timeoutForConnect_ = value;
}
int HttpClient::getTimeoutForConnect() const
{
	return timeoutForConnect_;
}

void HttpClient::setTimeoutForRead(int value)
{
	timeoutForRead_ = value;
}
int HttpClient::getTimeoutForRead() const
{
	return timeoutForRead_;
}

void HttpClient::send(const RequestPtr& request)
{
	ResponsePtr response = std::make_shared<HttpResponse>();
	threadPool_.run(std::bind(&HttpClient::processResponse, this, request, response));
}

void HttpClient::sendImmediate(const RequestPtr& request)
{
	ResponsePtr response = std::make_shared<HttpResponse>();
	auto t = std::thread(std::bind(&HttpClient::processResponse, this, request, response));
	t.detach();
}

void HttpClient::processResponse(const RequestPtr& request, ResponsePtr& response)
{
	switch (request->getRequestType())
	{
	case HttpRequest::Type::GET:
		doGet(request, response);
		break;
	case HttpRequest::Type::POST:
		doPost(request, response);
		break;
	case HttpRequest::Type::PUT:
		doPut(request, response);
		break;
	case HttpRequest::Type::DELETE:
		doPut(request, response);
		break;
	default:
		break;
	}

	auto callback = request->getResponseCallback();
	callback(request, response);
}

void HttpClient::doGet(const RequestPtr& request, ResponsePtr& response)
{
	char errorBuf[kErrorBufSize] = { 0 };
	auto responseData	= response->getResponseData();
	auto responseHeader = response->getResponseHeader();
	long responseCode	= -1;

	Curl curl;
	bool ok = curl.init(*this, request, responseData, responseHeader, errorBuf) &&
			  curl.setOption(CURLOPT_FOLLOWLOCATION, true) &&
			  curl.perform(&responseCode);

	response->setResponseCode(responseCode);
	if (ok)
	{
		response->setSucceed(true);
	}
	else
	{
		response->setSucceed(false);
		response->setErrorBuffer(errorBuf);
	}
}

void HttpClient::doPost(const RequestPtr& request, ResponsePtr& response)
{
	char errorBuf[kErrorBufSize] = { 0 };
	auto responseData	= response->getResponseData();
	auto responseHeader = response->getResponseHeader();
	auto postData		= request->getRequestData();
	auto postDataSize	= request->getRequestDataSize();
	long responseCode	= -1;

	Curl curl;
	bool ok = curl.init(*this, request, responseData, responseHeader, errorBuf) &&
			  curl.setOption(CURLOPT_POST, 1) &&
			  curl.setOption(CURLOPT_POSTFIELDS, postData) &&
			  curl.setOption(CURLOPT_POSTFIELDSIZE, postDataSize) &&
			  curl.perform(&responseCode);

	response->setResponseCode(responseCode);
	if (ok)
	{
		response->setSucceed(true);
	}
	else
	{
		response->setSucceed(false);
		response->setErrorBuffer(errorBuf);
	}
}

void HttpClient::doPut(const RequestPtr& request, ResponsePtr& response)
{
	auto responseData = response->getResponseData();
	auto responseHeader = response->getResponseHeader();
	char errorBuf[kErrorBufSize] = { 0 };
	auto requestData = request->getRequestData();
	auto requestDataSize = request->getRequestDataSize();
	long responseCode = -1;

	FILE* fp = nullptr;
	auto path = request->getUploadFilePath();
	size_t size = 0;
	if (!path.empty())
	{
		fp = fopen(path.data(), "rb");
		fseek(fp, 0, SEEK_END);
		size = ftell(fp);
	}

	Curl curl;
	curl.init(*this, request, responseData, responseHeader, errorBuf);
	//curl.setOption(CURLOPT_PUT, 1L);
	curl.setOption(CURLOPT_CUSTOMREQUEST, "PUT");
	curl.setOption(CURLOPT_VERBOSE, true);
	curl.setOption(CURLOPT_POSTFIELDS, requestData);
	curl.setOption(CURLOPT_POSTFIELDSIZE, requestDataSize);
	
	if (fp)
	{
		curl.setOption(CURLOPT_UPLOAD, 1L);
		curl.setOption(CURLOPT_READFUNCTION, readCallback);
		curl.setOption(CURLOPT_READDATA, fp);
		curl.setOption(CURLOPT_INFILESIZE_LARGE, (curl_off_t)size);
	}

	bool ok = curl.perform(&responseCode);

	response->setResponseCode(responseCode);
	if (ok)
	{
		response->setSucceed(true);
	}
	else
	{
		response->setSucceed(false);
		response->setErrorBuffer(errorBuf);
	}

	if (fp) fclose(fp);
}

void HttpClient::doDelete(const RequestPtr& request, ResponsePtr& response)
{
	char errorBuf[kErrorBufSize] = { 0 };
	auto responseData = response->getResponseData();
	auto responseHeader = response->getResponseHeader();
	auto postData = request->getRequestData();
	auto postDataSize = request->getRequestDataSize();
	long responseCode = -1;

	Curl curl;
	bool ok = curl.init(*this, request, responseData, responseHeader, errorBuf) &&
		curl.setOption(CURLOPT_CUSTOMREQUEST, "DELETE") &&
		curl.setOption(CURLOPT_FOLLOWLOCATION, true) &&
		curl.perform(&responseCode);

	response->setResponseCode(responseCode);
	if (ok)
	{
		response->setSucceed(true);
	}
	else
	{
		response->setSucceed(false);
		response->setErrorBuffer(errorBuf);
	}
}