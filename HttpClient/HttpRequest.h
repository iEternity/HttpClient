#pragma once
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <curl.h>

namespace xnet
{

class HttpRequest;
class HttpResponse;
using RequestCallback = std::function<void(const std::shared_ptr<HttpRequest>&, 
										   const std::shared_ptr<HttpResponse>&)>;
#ifdef DELETE
#undef DELETE
#endif

class HttpRequest
{
public:
	enum class Type
	{
		GET,
		POST,
		PUT,
		DELETE,
		UNKNOWN
	};

	HttpRequest() :
		requestType_(Type::UNKNOWN),
		pUserData_(nullptr)
	{

	}

	~HttpRequest()
	{
	}

	void setRequestType(const Type type)
	{
		requestType_ = type;
	}
	const Type	getRequestType() const
	{
		return requestType_;
	}

	void setUrl(const std::string& url)
	{
		url_ = url;
	}
	void setUrl(std::string&& url)
	{
		url_ = std::move(url);
	}
	const char* getUrl() const
	{
		return url_.data();
	}

	void setRequestData(const char* buffer, size_t len)
	{
		requestData_.assign(buffer, buffer + len);
	}
	void setRequestData(const std::string& str)
	{
		requestData_.assign(str.data(), str.data() + str.size());
	}

	const char* getRequestData() const
	{
		if (requestData_.empty()) return nullptr;

		return &requestData_.front();
	}

	size_t getRequestDataSize() const
	{
		return requestData_.size();
	}

	void setUploadFilePath(const std::string& path)
	{
		uploadFilePath_ = path;
	}
	void setUploadFilePath(std::string&& path)
	{
		uploadFilePath_ = std::move(path);
	}
	const std::string& getUploadFilePath() const
	{
		return uploadFilePath_;
	}

	void setTag(const std::string& tag)
	{
		tag_ = tag;
	}
	void setTag(std::string&& tag)
	{
		tag_ = std::move(tag);
	}
	const std::string& getTag() const
	{
		return tag_;
	}

	void setUserData(std::shared_ptr<void> data)
	{
		pUserData_ = data;
	}
	const void* getUserData() const
	{
		return pUserData_.get();
	}

	void setCallback(const RequestCallback& cb)
	{
		callback_ = cb;
	}
	void setCallback(RequestCallback&& cb)
	{
		callback_ = std::move(cb);
	}
	const RequestCallback& getResponseCallback() const
	{
		return callback_;
	}

	void setHeaders(const std::vector<std::string>& headers)
	{
		headers_ = headers;
	}
	void setHeaders(std::vector<std::string>&& headers)
	{
		headers_ = std::move(headers);
	}
	const std::vector<std::string> getHeaders() const
	{
		return headers_;
	}

private:
	Type						requestType_;
	std::string					url_;
	std::vector<char>			requestData_;
	std::string					tag_;
	std::shared_ptr<void>		pUserData_;
	RequestCallback				callback_;
	std::vector<std::string>	headers_;
	std::string					uploadFilePath_;
};

}