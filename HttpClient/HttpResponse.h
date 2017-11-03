#pragma once
#include "HttpRequest.h"

namespace xnet
{

class HttpResponse
{
public:
	HttpResponse() :
		succeed_(false),
		responseCode_(-1)
	{

	}

	~HttpResponse() = default;

	void setSucceed(const bool isSucceed)
	{
		succeed_ = isSucceed;
	}
	bool isSucceed() const
	{
		return succeed_;
	}

	void setResponseData(const std::vector<char>& data)
	{
		responseData_ = data;
	}
	void setResponseData(std::vector<char>&& data)
	{
		responseData_ = std::move(data);
	}
	std::vector<char>* getResponseData()
	{
		return &responseData_;
	}

	void setResponseHeader(const std::vector<char>& responseHeader)
	{
		responseHeader_ = responseHeader;
	}
	void setResponseHeader(std::vector<char>&& responseHeader)
	{
		responseHeader_ = std::move(responseHeader);
	}
	std::vector<char>* getResponseHeader()
	{
		return &responseHeader_;
	}

	void setResponseCode(long code)
	{
		responseCode_ = code;
	}
	long getResponseCode() const
	{
		return responseCode_;
	}

	void setResponseDataString(const std::string& str)
	{
		responseDataString_ = str;
	}
	void setResponseDataString(std::string&& str)
	{
		responseDataString_ = std::move(str);
	}
	const std::string& getResponseDataString() const
	{
		return responseDataString_;
	}

	std::string responseDataAsString() const
	{
		if (responseData_.empty()) return "receive data is empty!";
		return std::string(&*responseData_.begin(), responseData_.size());
	}

	void setErrorBuffer(const std::string& errBuf)
	{
		errorBuffer_ = errBuf;
	}
	void setErrorBuffer(std::string&& errBuf)
	{
		errorBuffer_ = std::move(errBuf);
	}
	const std::string& gerErrorBuffer() const
	{
		return errorBuffer_;
	}

private:
	bool				succeed_;
	std::vector<char>	responseData_;
	std::vector<char>	responseHeader_;
	long			    responseCode_;
	std::string			responseDataString_;
	std::string			errorBuffer_;
};

}