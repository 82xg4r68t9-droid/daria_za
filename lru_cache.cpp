// lru_cache.cpp
#include "lru_cache.h"
#include <vector>

LRUCache::LRUCache(size_t capacity) : capacity_(capacity) {
}

size_t LRUCache::Size() const {
    return items_.size();
}

size_t LRUCache::Capacity() const {
    return capacity_;
}

void LRUCache::Clear() noexcept {
    items_.clear();
    non_pinned_items_.clear();
    dict_.clear();
}

std::optional<int> LRUCache::Get(const std::string& key) {
    auto it = dict_.find(key);
    if (it == dict_.end()) {
        return std::nullopt;
    }
    auto& list_it = it->second.first;
    auto& non_pinned_opt = it->second.second;

    items_.splice(items_.begin(), items_, list_it);

    if (!list_it->pinned) {
        if (non_pinned_opt.has_value()) {
            non_pinned_items_.erase(*non_pinned_opt);
        }
        non_pinned_items_.push_front(list_it);
        non_pinned_opt = non_pinned_items_.begin();
    }

    return list_it->value;
}

bool LRUCache::Put(const std::string& key, int value) {
    if (capacity_ == 0) {
        return false;
    }

    auto it = dict_.find(key);
    if (it != dict_.end()) {
        auto& list_it = it->second.first;
        auto& non_pinned_opt = it->second.second;

        if (list_it->pinned) {
            return false;
        }

        list_it->value = value;
        items_.splice(items_.begin(), items_, list_it);

        if (non_pinned_opt.has_value()) {
            non_pinned_items_.erase(*non_pinned_opt);
        }
        non_pinned_items_.push_front(list_it);
        non_pinned_opt = non_pinned_items_.begin();

        return false;
    }

    if (items_.size() >= capacity_) {
        if (non_pinned_items_.empty()) {
            return false;
        }
        auto oldest = non_pinned_items_.back();
        dict_.erase(oldest->key);
        non_pinned_items_.pop_back();
        items_.erase(oldest);
    }

    items_.push_front({key, value, false});
    auto new_it = items_.begin();
    non_pinned_items_.push_front(new_it);
    dict_[key] = std::make_pair(new_it, non_pinned_items_.begin());
    return true;
}

bool LRUCache::Erase(const std::string& key) {
    auto it = dict_.find(key);
    if (it == dict_.end()) {
        return false;
    }
    auto& list_it = it->second.first;
    auto& non_pinned_opt = it->second.second;

    if (non_pinned_opt.has_value()) {
        non_pinned_items_.erase(*non_pinned_opt);
    }
    items_.erase(list_it);
    dict_.erase(it);
    return true;
}

bool LRUCache::Pin(const std::string& key) {
    auto it = dict_.find(key);
    if (it == dict_.end()) {
        return false;
    }
    auto& list_it = it->second.first;
    auto& non_pinned_opt = it->second.second;

    if (list_it->pinned) {
        return false;
    }

    list_it->pinned = true;
    if (non_pinned_opt.has_value()) {
        non_pinned_items_.erase(*non_pinned_opt);
        non_pinned_opt = std::nullopt;
    }
    return true;
}

bool LRUCache::Unpin(const std::string& key) {
    auto it = dict_.find(key);
    if (it == dict_.end()) {
        return false;
    }
    auto& list_it = it->second.first;
    auto& non_pinned_opt = it->second.second;

    if (!list_it->pinned) {
        return false;
    }

    list_it->pinned = false;
    items_.splice(items_.begin(), items_, list_it);
    non_pinned_items_.push_front(list_it);
    non_pinned_opt = non_pinned_items_.begin();
    return true;
}

void LRUCache::Merge(LRUCache& other) {
    if (this == &other) {
        return;
    }

    for (auto oit = other.items_.begin(); oit != other.items_.end();) {
        if (dict_.find(oit->key) != dict_.end()) {
            auto other_map_it = other.dict_.find(oit->key);
            if (other_map_it != other.dict_.end()) {
                if (other_map_it->second.second.has_value()) {
                    other.non_pinned_items_.erase(*other_map_it->second.second);
                }
                other.dict_.erase(other_map_it);
            }
            oit = other.items_.erase(oit);
        } else {
            ++oit;
        }
    }

    if (other.items_.empty()) {
        return;
    }

    std::vector<std::pair<std::string, std::pair<int, bool>>> elements;
    elements.reserve(other.items_.size());
    for (auto it = other.items_.rbegin(); it != other.items_.rend(); ++it) {
        elements.emplace_back(it->key, std::make_pair(it->value, it->pinned));
    }

    other.Clear();

    for (const auto& elem : elements) {
        if (Put(elem.first, elem.second.first) && elem.second.second) {
            Pin(elem.first);
        }
    }
}