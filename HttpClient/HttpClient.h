#pragma once
#include <mutex>
#include <string>
#include <memory>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpCookie.h"
#include "ThreadPool.h"

namespace xnet
{

class HttpClient
{
	using LockGuard		= std::lock_guard<std::mutex>;
	using RequestPtr	= std::shared_ptr<HttpRequest>;
	using ResponsePtr	= std::shared_ptr<HttpResponse>;

public:
	static HttpClient* getInstance()
	{
		return instance_;
	}

	void	enableCookies(const std::string& cookieFile = std::string());
	const	std::string& getCookieFilename() const;

	void	setSSLVerification(const std::string& caFile);
	void	setSSLVerification(std::string&& caFile);
	const	std::string& getSSLVerification() const;

	void	setTimeoutForConnect(int value);
	int		getTimeoutForConnect() const;

	void	setTimeoutForRead(int value);
	int		getTimeoutForRead() const;

	void	send(const RequestPtr& request);
	void	sendImmediate(const RequestPtr& request);

	const HttpCookie* getCookie() const { return cookie_; }

private:
	HttpClient();
	~HttpClient();
	HttpClient(const HttpClient&) = delete;
	HttpClient& operator=(const HttpClient&) = delete;

	void	processResponse(const RequestPtr& request, ResponsePtr& response);
	void	doGet(const RequestPtr& request, ResponsePtr& response);
	void	doPost(const RequestPtr& request, ResponsePtr& response);
	void	doPut(const RequestPtr& request, ResponsePtr& response);
	void	doDelete(const RequestPtr& request, ResponsePtr& response);

private:
	static const int	kNumThreads		= 4;
	static const int	kErrorBufSize	= 256;

	static HttpClient*	instance_;

	mutable std::mutex	cookieFileMutex_;
	std::string			cookieFilename_;

	mutable std::mutex	sslCaFileMutex_;
	std::string			sslCaFilename_;

	int					timeoutForConnect_;
	int					timeoutForRead_;

	HttpCookie*			cookie_;

	ThreadPool			threadPool_;
};

}