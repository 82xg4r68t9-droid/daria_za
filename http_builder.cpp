#include "http_builder.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <sstream>

namespace {
bool CaseInsensitiveEqual(std::string_view a, std::string_view b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (size_t i = 0; i < a.size(); ++i) {
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i]))) {
            return false;
        }
    }
    return true;
}

bool IsHostHeader(std::string_view name) {
    return CaseInsensitiveEqual(name, "Host");
}

bool IsContentLengthHeader(std::string_view name) {
    return CaseInsensitiveEqual(name, "Content-Length");
}

constexpr std::array<char, 16> HexTable = {'0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

constexpr int KLowNibbleMask = 0xF;

std::string PercentEncode(std::string_view s) {
    std::string result;
    for (unsigned char byte : s) {
        if (('A' <= byte && byte <= 'Z') || ('a' <= byte && byte <= 'z') || ('0' <= byte && byte <= '9') ||
            byte == '-' || byte == '_' || byte == '.' || byte == '~') {
            result += static_cast<char>(byte);
        } else {
            result += '%';
            result += HexTable[byte >> 4];
            result += HexTable[byte & KLowNibbleMask];
        }
    }
    return result;
}

std::string_view MethodToString(HttpMethod m) {
    switch (m) {
        case HttpMethod::Get:
            return "GET";
        case HttpMethod::Head:
            return "HEAD";
        case HttpMethod::Post:
            return "POST";
        case HttpMethod::Put:
            return "PUT";
        case HttpMethod::Delete:
            return "DELETE";
        case HttpMethod::Patch:
            return "PATCH";
        case HttpMethod::Options:
            return "OPTIONS";
        default:
            return "UNKNOWN";
    }
}
}  // namespace

std::string_view HttpHeaderCollection::GetValue(std::string_view name) const {
    for (const auto& [key, val] : data_) {
        if (CaseInsensitiveEqual(key, name)) {
            return val;
        }
    }
    return {};
}

void HttpHeaderCollection::Set(std::string_view name, std::string_view value) {
    Remove(name);
    data_.emplace(std::string(name), std::string(value));
}

void HttpHeaderCollection::Remove(std::string_view name) {
    for (auto it = data_.begin(); it != data_.end();) {
        if (CaseInsensitiveEqual(it->first, name)) {
            it = data_.erase(it);
        } else {
            ++it;
        }
    }
}

std::size_t HttpHeaderCollection::Size() const {
    return data_.size();
}

bool HttpHeaderCollection::Contains(std::string_view name) const {
    for (const auto& [key, _] : data_) {
        if (CaseInsensitiveEqual(key, name)) {
            return true;
        }
    }
    return false;
}

const HttpHeaderCollection::Storage& HttpHeaderCollection::Items() const {
    return data_;
}

std::string HttpRequest::ToString() const {
    std::ostringstream oss;
    oss << MethodToString(method_) << ' ' << target_ << " HTTP/1.1\r\n";
    oss << "Host: " << host_;
    if (port_) {
        oss << ':' << *port_;
    }
    oss << "\r\n";
    for (const auto& [name, val] : headers_.Items()) {
        oss << name << ": " << val << "\r\n";
    }
    oss << "\r\n";
    if (body_) {
        oss << *body_;
    }
    return oss.str();
}

HttpMethod HttpRequest::GetMethod() const {
    return method_;
}
std::string_view HttpRequest::GetTarget() const {
    return target_;
}
std::string_view HttpRequest::GetHost() const {
    return host_;
}
std::optional<std::uint16_t> HttpRequest::GetPort() const {
    return port_;
}
const HttpHeaderCollection& HttpRequest::GetHeaders() const {
    return headers_;
}
std::optional<std::string_view> HttpRequest::GetBody() const {
    return body_ ? std::optional<std::string_view>(*body_) : std::nullopt;
}

HttpRequest::Builder::Builder() {
    Reset();
}

HttpRequest::Builder& HttpRequest::Builder::Reset() {
    method_ = HttpMethod::Get;
    target_.clear();
    host_.clear();
    port_.reset();
    headers_ = HttpHeaderCollection{};
    body_.reset();
    query_.clear();
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetMethod(HttpMethod method) {
    method_ = method;
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetTarget(std::string_view target) {
    if (target.empty()) {
        target_ = "/";
    } else {
        target_ = (target[0] == '/') ? std::string(target) : '/' + std::string(target);
    }
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::Get(std::string_view target) {
    method_ = HttpMethod::Get;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Head(std::string_view target) {
    method_ = HttpMethod::Head;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Post(std::string_view target) {
    method_ = HttpMethod::Post;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Put(std::string_view target) {
    method_ = HttpMethod::Put;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Delete(std::string_view target) {
    method_ = HttpMethod::Delete;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Patch(std::string_view target) {
    method_ = HttpMethod::Patch;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::Options(std::string_view target) {
    method_ = HttpMethod::Options;
    return SetTarget(target);
}

HttpRequest::Builder& HttpRequest::Builder::SetHost(std::string_view host) {
    host_ = host;
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetPort(std::uint16_t port) {
    port_ = port;
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetQuery(std::string_view key, std::string_view value) {
    query_.emplace_back(std::string(key), std::string(value));
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::ClearQuery() {
    query_.clear();
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::RemoveQuery(std::string_view key) {
    std::string key_str(key);
    auto it = std::remove_if(query_.begin(), query_.end(), [&](const auto& p) { return p.first == key_str; });
    query_.erase(it, query_.end());
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetHeader(std::string_view name, std::string_view value) {
    if (IsHostHeader(name)) {
        host_ = value;
    } else {
        headers_.Set(name, value);
    }
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::RemoveHeader(std::string_view name) {
    if (IsHostHeader(name)) {
        host_.clear();
        port_.reset();
    } else {
        headers_.Remove(name);
    }
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetBody(std::string_view data) {
    body_ = std::string(data);
    return *this;
}

HttpRequest::Builder& HttpRequest::Builder::SetNoBody() {
    body_.reset();
    return *this;
}

HttpRequest HttpRequest::Builder::Build() {
    std::string processed_target = target_.empty() ? "/" : target_;
    if (!query_.empty()) {
        processed_target += '?';
        bool is_first = true;
        for (const auto& [param_key, param_val] : query_) {
            if (!is_first) {
                processed_target += '&';
            }
            is_first = false;
            processed_target += PercentEncode(param_key);
            processed_target += '=';
            processed_target += PercentEncode(param_val);
        }
    }

    HttpHeaderCollection result_headers;
    for (const auto& [name, value] : headers_.Items()) {
        if (!IsHostHeader(name) && !IsContentLengthHeader(name)) {
            result_headers.Set(name, value);
        }
    }

    if (body_) {
        result_headers.Set("Content-Length", std::to_string(body_->size()));
    }

    HttpRequest request;
    request.method_ = method_;
    request.target_ = std::move(processed_target);
    request.host_ = host_;
    request.port_ = port_;
    request.headers_ = std::move(result_headers);
    request.body_ = body_;
    return request;
}