#include <gtest/gtest.h>

#include "types.h"
#include "util/memory.h"

namespace toggl {

using toggl::guid;
static int modelCounter = 0;

class TestModel {
public:
    TestModel(ProtectedBase*, guid uuid, uint64_t id)
        : uuid_(uuid)
        , id_(id)
    {
        modelCounter++;
    }
    ~TestModel() {
        modelCounter--;
    }
    std::string ModelName() {
        return "Test";
    }
    guid GUID() {
        return uuid_;
    }
    id_t ID() {
        return id_;
    }
    guid uuid_;
    uint64_t id_;
};

TEST(ProtectedContainer, Init) {
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    ASSERT_EQ(container.begin(), container.end());
    ASSERT_EQ(container.cbegin(), container.cend());
    ASSERT_EQ(modelCounter, 0);
}

TEST(ProtectedContainer, EmptySearch) {
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);

    auto first = *container.begin();
    ASSERT_FALSE(first);
    ASSERT_EQ(modelCounter, 0);
}

TEST(ProtectedContainer, Insert) {
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("abc"), 123);
    ASSERT_EQ(container.size(), 1);

    auto first = *container.begin();
    ASSERT_EQ(first->GUID(), "abc");
    first = container[size_t(0)];
    ASSERT_EQ(first->GUID(), "abc");
    auto byUuid = container["abc"];
    ASSERT_EQ(byUuid->GUID(), "abc");
    auto byId = container.byID(123);
    ASSERT_EQ(byId->GUID(), "abc");
    ASSERT_EQ(modelCounter, 1);
}

TEST(ProtectedContainer, InsertAndWrongSearch) {
    ASSERT_EQ(modelCounter, 0);
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("abc"), 123);
    ASSERT_EQ(container.size(), 1);

    auto second = *(++container.begin());
    ASSERT_FALSE(second);
    second = container[size_t(1)];
    ASSERT_FALSE(second);
    auto byUuid = container["cde"];
    ASSERT_FALSE(byUuid);
    auto byId = container.byID(321);
    ASSERT_FALSE(byId);
    ASSERT_EQ(modelCounter, 1);
}

TEST(ProtectedContainer, InsertAndRemoveByUuid) {
    ASSERT_EQ(modelCounter, 0);
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("1"), 100);
    ASSERT_EQ(modelCounter, 1);
    ASSERT_EQ(container.size(), 1);
    container.remove("1");
    ASSERT_EQ(container.size(), 0);

    auto first = *container.begin();
    ASSERT_FALSE(first);
    first = container[size_t(0)];
    ASSERT_FALSE(first);
    auto byUuid = container[guid("1")];
    ASSERT_FALSE(byUuid);
    auto byId = container.byID(100);
    ASSERT_FALSE(byId);
    ASSERT_EQ(modelCounter, 0);
}

TEST(ProtectedContainer, InsertAndRemoveByIterator) {
    ASSERT_EQ(modelCounter, 0);
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("1"), 100);
    ASSERT_EQ(container.size(), 1);
    ASSERT_EQ(modelCounter, 1);
    auto it = container.begin();
    ASSERT_NE(it, container.end());
    container.erase(it);
    ASSERT_EQ(it, container.end());
    ASSERT_EQ(container.size(), 0);
    ASSERT_EQ(modelCounter, 0);

    auto first = *container.begin();
    ASSERT_FALSE(first);
    first = container[size_t(0)];
    ASSERT_FALSE(first);
    auto byUuid = container[guid("1")];
    ASSERT_FALSE(byUuid);
    auto byId = container.byID(100);
    ASSERT_FALSE(byId);
}

TEST(ProtectedContainer, InsertAndRemoveMultiple) {
    ASSERT_EQ(modelCounter, 0);
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("1"), 100);
    ASSERT_EQ(container.size(), 1);
    container.create(guid("2"), 200);
    ASSERT_EQ(container.size(), 2);
    container.create(guid("3"), 300);
    ASSERT_EQ(container.size(), 3);
    container.create(guid("4"), 400);
    ASSERT_EQ(container.size(), 4);
    container.create(guid("5"), 500);
    ASSERT_EQ(container.size(), 5);
    ASSERT_EQ(modelCounter, 5);

    container.remove("3");
    ASSERT_EQ(container.size(), 4);
    auto item = container[guid("3")];
    ASSERT_FALSE(item);
    item = container[guid("1")];
    ASSERT_TRUE(item);
    item = container[guid("2")];
    ASSERT_TRUE(item);
    item = container[guid("4")];
    ASSERT_TRUE(item);
    item = container[guid("5")];
    ASSERT_TRUE(item);
    container.remove("1");
    ASSERT_EQ(container.size(), 3);
    container.remove("5");
    ASSERT_EQ(container.size(), 2);
    container.remove("4");
    ASSERT_EQ(container.size(), 1);
    container.remove("2");
    ASSERT_EQ(container.size(), 0);
    ASSERT_EQ(modelCounter, 0);
}

TEST(ProtectedContainer, InsertAndClearMultiple) {
    ASSERT_EQ(modelCounter, 0);
    ProtectedContainer<TestModel> container { nullptr };
    ASSERT_EQ(container.size(), 0);
    container.create(guid("1"), 100);
    ASSERT_EQ(container.size(), 1);
    container.create(guid("2"), 200);
    ASSERT_EQ(container.size(), 2);
    container.create(guid("3"), 300);
    ASSERT_EQ(container.size(), 3);
    container.create(guid("4"), 400);
    ASSERT_EQ(container.size(), 4);
    container.create(guid("5"), 500);
    ASSERT_EQ(container.size(), 5);
    ASSERT_EQ(modelCounter, 5);
    container.clear();
    ASSERT_EQ(container.size(), 0);
    ASSERT_EQ(modelCounter, 0);
}

} // namespace toggl
/*
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
*/
