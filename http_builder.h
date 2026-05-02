#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

enum class HttpMethod { Get, Head, Post, Put, Delete, Patch, Options };

class HttpHeaderCollection {
public:
    using Storage = std::unordered_map<std::string, std::string>;
    std::string_view GetValue(std::string_view name) const;
    void Set(std::string_view name, std::string_view value);
    void Remove(std::string_view name);
    std::size_t Size() const;
    bool Contains(std::string_view name) const;
    const Storage& Items() const;

private:
    Storage data_;
};

class HttpRequest {
public:
    std::string ToString() const;
    HttpMethod GetMethod() const;
    std::string_view GetTarget() const;
    std::string_view GetHost() const;
    std::optional<std::uint16_t> GetPort() const;
    const HttpHeaderCollection& GetHeaders() const;
    std::optional<std::string_view> GetBody() const;

    class Builder {
    public:
        Builder();
        Builder& Reset();
        Builder& SetMethod(HttpMethod method);
        Builder& SetTarget(std::string_view target);
        Builder& Get(std::string_view target = "/");
        Builder& Head(std::string_view target = "/");
        Builder& Post(std::string_view target = "/");
        Builder& Put(std::string_view target = "/");
        Builder& Delete(std::string_view target = "/");
        Builder& Patch(std::string_view target = "/");
        Builder& Options(std::string_view target = "/");
        Builder& SetHost(std::string_view host);
        Builder& SetPort(std::uint16_t port);
        Builder& SetQuery(std::string_view key, std::string_view value);
        Builder& ClearQuery();
        Builder& RemoveQuery(std::string_view key);
        Builder& SetHeader(std::string_view name, std::string_view value);
        Builder& RemoveHeader(std::string_view name);
        Builder& SetBody(std::string_view data);
        Builder& SetNoBody();
        HttpRequest Build();

    private:
        HttpMethod method_ = HttpMethod::Get;
        std::string target_;
        std::string host_;
        std::optional<std::uint16_t> port_;
        HttpHeaderCollection headers_;
        std::optional<std::string> body_;
        std::vector<std::pair<std::string, std::string>> query_;
    };

private:
    HttpRequest() = default;
    friend class Builder;
    HttpMethod method_;
    std::string target_;
    std::string host_;
    std::optional<std::uint16_t> port_;
    HttpHeaderCollection headers_;
    std::optional<std::string> body_;
};