#include <catch.hpp>

#include "lru_cache.h"

TEST_CASE("Basic") {
    LRUCache cache(3);

    SECTION("Empty operations") {
        REQUIRE(cache.Size() == 0);
        REQUIRE(cache.Capacity() == 3);
        REQUIRE(!cache.Get("nothing").has_value());
        REQUIRE(cache.Erase("nothing") == false);
    }

    SECTION("Basic put and get") {
        REQUIRE(cache.Put("1", 100) == true);
        REQUIRE(cache.Size() == 1);

        auto value = cache.Get("1");

        REQUIRE(value.has_value());
        REQUIRE(*value == 100);
    }

    SECTION("Value updates preserves order") {
        cache.Put("1", 1);
        cache.Put("2", 2);
        cache.Put("3", 3);

        // update existing key
        REQUIRE(cache.Put("1", 100) == false);  // false because key exists
        REQUIRE(cache.Get("1").value() == 100);

        // key '1' should now be most recent
        cache.Put("4", 4);  // should pop key '2' (oldest)

        REQUIRE(!cache.Get("2").has_value());
        REQUIRE(cache.Get("1").has_value());
        REQUIRE(cache.Get("3").has_value());
        REQUIRE(cache.Get("4").has_value());
    }
}

TEST_CASE("Pop operation") {
    SECTION("Exact capacity behavior") {
        LRUCache cache(2);

        cache.Put("a", 1);
        cache.Put("b", 2);

        REQUIRE(cache.Size() == 2);

        cache.Put("c", 3);  // should pop 'a'

        REQUIRE(!cache.Get("a").has_value());
        REQUIRE(cache.Get("b").value() == 2);
        REQUIRE(cache.Get("c").value() == 3);
    }

    SECTION("Access pattern affects to pop") {
        LRUCache cache(3);

        cache.Put("a", 1);
        cache.Put("b", 2);
        cache.Put("c", 3);

        // makes 'a' most recently used
        cache.Get("a");

        // add new element - should evict 'b' (least recently used)
        cache.Put("d", 4);

        REQUIRE(!cache.Get("b").has_value());
        REQUIRE(cache.Get("a").has_value());
        REQUIRE(cache.Get("c").has_value());
        REQUIRE(cache.Get("d").has_value());
    }

    SECTION("Complex access pattern") {
        LRUCache cache(4);

        cache.Put("a", 1);
        cache.Put("b", 2);
        cache.Put("c", 3);
        cache.Put("d", 4);

        // access sequence: 'b', 'c', 'd', 'a'
        cache.Get("b");
        cache.Get("c");
        cache.Get("d");
        cache.Get("a");

        // now order should be: 'a', 'd', 'c', 'b'
        cache.Put("e", 5);  // should pop 'b'

        REQUIRE(!cache.Get("b").has_value());
        REQUIRE(cache.Get("a").has_value());
        REQUIRE(cache.Get("c").has_value());
        REQUIRE(cache.Get("d").has_value());
        REQUIRE(cache.Get("e").has_value());
    }
}

TEST_CASE("Pinning") {
    LRUCache cache(3);

    SECTION("Pin protects from pop") {
        cache.Put("pinned", 1);
        cache.Put("1", 2);
        cache.Put("2", 3);

        REQUIRE(cache.Pin("pinned") == true);

        // this should pop '1' (oldest unpinned)
        cache.Put("new", 4);

        REQUIRE(!cache.Get("1").has_value());
        REQUIRE(cache.Get("pinned").has_value());
        REQUIRE(cache.Get("2").has_value());
        REQUIRE(cache.Get("new").has_value());
    }

    SECTION("Multiple pins behavior") {
        cache.Put("pinned1", 1);
        cache.Put("pinned2", 2);
        cache.Put("3", 3);

        cache.Pin("pinned1");
        cache.Pin("pinned2");

        REQUIRE(cache.Put("1", 4) == true);
        REQUIRE(!cache.Get("3").has_value());
        REQUIRE(cache.Get("1").has_value());

        REQUIRE(cache.Put("2", 5) == true);
        REQUIRE(!cache.Get("1").has_value());
        REQUIRE(cache.Get("2").has_value());

        REQUIRE(cache.Get("pinned1").has_value());
        REQUIRE(cache.Get("pinned2").has_value());
    }

    SECTION("Pin/unpin lifecycle") {
        cache.Put("key", 1);

        REQUIRE(cache.Pin("key") == true);
        REQUIRE(cache.Pin("key") == false);  // already pinned

        REQUIRE(cache.Unpin("key") == true);
        REQUIRE(cache.Unpin("key") == false);  // already unpinned

        cache.Put("a", 2);
        cache.Put("b", 3);
        cache.Put("c", 4);  // should pop 'key'

        REQUIRE(!cache.Get("key").has_value());
    }

    SECTION("Pin affects LRU order") {
        cache.Put("a", 1);
        cache.Put("b", 2);
        cache.Put("c", 3);

        cache.Pin("b");
        cache.Get("a");

        // should pop 'c', not 'a'
        cache.Put("d", 4);

        REQUIRE(!cache.Get("c").has_value());
        REQUIRE(cache.Get("a").has_value());
        REQUIRE(cache.Get("b").has_value());
        REQUIRE(cache.Get("d").has_value());
    }
}

TEST_CASE("Merging") {
    SECTION("Basic merge") {
        LRUCache cache1(5);

        cache1.Put("1", 1);
        cache1.Put("2", 2);
        cache1.Pin("1");

        LRUCache cache2(3);

        cache2.Put("3", 3);
        cache2.Put("4", 4);

        cache1.Merge(cache2);

        REQUIRE(cache1.Size() == 4);
        REQUIRE(*cache1.Get("1") == 1);
        REQUIRE(*cache1.Get("2") == 2);
        REQUIRE(*cache1.Get("3") == 3);
        REQUIRE(*cache1.Get("4") == 4);
    }

    SECTION("Merge with duplicates") {
        LRUCache cache1(3);

        cache1.Put("common", 100);
        cache1.Put("unique1", 1);

        LRUCache cache2(3);

        cache2.Put("common", 200);  // different value
        cache2.Put("unique2", 2);

        cache1.Merge(cache2);

        // original value should be preserved
        REQUIRE(cache1.Get("common").value() == 100);
        REQUIRE(cache1.Get("unique1").has_value());
        REQUIRE(cache1.Get("unique2").has_value());
        REQUIRE(cache1.Size() == 3);
    }

    SECTION("Merge with pop") {
        LRUCache cache1(3);

        cache1.Put("a", 1);

        LRUCache cache2(3);

        cache2.Put("b", 2);
        cache2.Put("c", 3);
        cache2.Put("d", 4);

        cache1.Merge(cache2);  // should pop one element

        REQUIRE(cache1.Size() == 3);
        REQUIRE(!cache1.Get("a").has_value());  // 'a' was popped (oldest)
        REQUIRE(cache1.Get("b").has_value());
        REQUIRE(cache1.Get("c").has_value());
        REQUIRE(cache1.Get("d").has_value());
    }

    SECTION("Self-merge") {
        LRUCache cache(3);

        cache.Put("key", 1);

        cache.Merge(cache);  // should do nothing

        REQUIRE(cache.Size() == 1);
        REQUIRE(cache.Get("key").value() == 1);
    }
}

TEST_CASE("Edge cases") {
    SECTION("Capacity=0 cache") {
        LRUCache cache(0);

        REQUIRE(cache.Put("key", 1) == false);
        REQUIRE(!cache.Get("key").has_value());
        REQUIRE(cache.Erase("key") == false);
        REQUIRE(cache.Size() == 0);
    }

    SECTION("Capacity=1 cache") {
        LRUCache cache(1);

        REQUIRE(cache.Put("first", 1) == true);
        REQUIRE(cache.Get("first").value() == 1);

        REQUIRE(cache.Put("second", 2) == true);  // pops 'first'
        REQUIRE(!cache.Get("first").has_value());
        REQUIRE(cache.Get("second").value() == 2);
    }

    SECTION("Mass operations") {
        LRUCache cache(1000);

        for (int i = 0; i < 1000; ++i) {
            REQUIRE(cache.Put("key" + std::to_string(i), i) == true);
        }

        REQUIRE(cache.Size() == 1000);

        for (int i = 0; i < 100; ++i) {
            auto val = cache.Get("key" + std::to_string(i * 10));
            REQUIRE(val.has_value());
            REQUIRE(*val == i * 10);
        }

        for (int i = 1000; i < 1100; ++i) {
            cache.Put("key" + std::to_string(i), i);
        }

        REQUIRE(cache.Size() == 1000);

        // first 100 should be popped (except those accessed)
        int found = 0;
        for (int i = 0; i < 100; ++i) {
            if (cache.Get("key" + std::to_string(i)).has_value()) {
                found++;
            }
        }
        REQUIRE(found < 20);  // most should be popped
    }
}

TEST_CASE("Error Conditions") {
    LRUCache cache(2);

    SECTION("Invalid operations") {
        REQUIRE(cache.Pin("nothing") == false);
        REQUIRE(cache.Unpin("nothing") == false);
        REQUIRE(cache.Erase("nothing") == false);

        cache.Put("key", 1);

        REQUIRE(cache.Pin("key") == true);
        REQUIRE(cache.Pin("key") == false);  // already pinned
        REQUIRE(cache.Unpin("key") == true);
        REQUIRE(cache.Unpin("key") == false);  // already unpinned
    }

    SECTION("Put on full cache with only pinned") {
        cache.Put("pinned1", 1);
        cache.Put("pinned2", 2);
        cache.Pin("pinned1");
        cache.Pin("pinned2");

        REQUIRE(cache.Put("new", 3) == false);
        REQUIRE(!cache.Get("new").has_value());
    }
}

TEST_CASE("Memory Management") {
    SECTION("Clear operation") {
        LRUCache cache(3);

        cache.Put("a", 1);
        cache.Put("b", 2);
        cache.Pin("a");

        cache.Clear();

        REQUIRE(cache.Size() == 0);
        REQUIRE(!cache.Get("a").has_value());
        REQUIRE(!cache.Get("b").has_value());

        // should be reusable after clear
        REQUIRE(cache.Put("new", 100) == true);
        REQUIRE(cache.Get("new").value() == 100);
    }
}

TEST_CASE("Complex Scenarios") {
    SECTION("Mixed pinned and unpinned pops") {
        LRUCache cache(4);

        cache.Put("a", 1);
        cache.Put("b", 2);
        cache.Put("c", 3);
        cache.Put("d", 4);

        cache.Pin("b");
        cache.Pin("d");

        cache.Get("a");
        cache.Get("c");

        cache.Put("e", 5);
        cache.Put("f", 6);

        REQUIRE(!cache.Get("a").has_value());  // popped
        REQUIRE(!cache.Get("c").has_value());  // popped
        REQUIRE(cache.Get("b").has_value());   // pinned
        REQUIRE(cache.Get("d").has_value());   // pinned
        REQUIRE(cache.Get("e").has_value());
        REQUIRE(cache.Get("f").has_value());
    }

    SECTION("Random operation stress test") {
        LRUCache cache(50);
        std::vector<std::string> keys;

        for (int i = 0; i < 100; ++i) {
            keys.push_back("key" + std::to_string(i));
        }

        std::random_device rd;
        std::mt19937 gen(rd());

        for (int op = 0; op < 1000; ++op) {
            int action = gen() % 100;
            const std::string& key = keys[gen() % keys.size()];

            if (action < 40) {  // 40% put
                cache.Put(key, op);
            } else if (action < 70) {  // 30% get
                cache.Get(key);
            } else if (action < 85) {  // 15% pin
                cache.Pin(key);
            } else if (action < 95) {  // 10% unpin
                cache.Unpin(key);
            } else {  // 5% erase
                cache.Erase(key);
            }

            REQUIRE(cache.Size() <= cache.Capacity());
        }

        // cache should still be functional
        REQUIRE(cache.Put("final", 999) == true);
        REQUIRE(cache.Get("final").value() == 999);
    }
}
