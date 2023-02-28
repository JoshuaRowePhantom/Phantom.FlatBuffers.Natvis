#include "DebuggerTestFlatBuffers_generated.h"
#include <gtest/gtest.h>
#include <memory>

namespace Phantom::FlatBuffers::Natvis::Test
{
using namespace FlatBuffers;

TEST(DebuggerTests, Step_into_this_and_look_at_visualization_in_VisualStudio)
{
    TestTableT topLevel;
    
    auto& subTable1 = topLevel.sub_table = std::make_unique<SubTableT>();
    subTable1->value = 1;

    auto& subTable2 = topLevel.vector_table.emplace_back(std::make_unique<SubTableT>());
    subTable2->value = 2;
    auto& subTable3 = topLevel.vector_table.emplace_back(std::make_unique<SubTableT>());
    subTable3->value = 3;

    topLevel.string_value = "hello world";
    topLevel.byte_vector.push_back(1);
    topLevel.byte_vector.push_back(2);
    topLevel.byte_vector.push_back(3);
    topLevel.double_vector.push_back(0.5);
    topLevel.double_vector.push_back(0.75);
    topLevel.double_vector.push_back(0.885);
    
    flatbuffers::FlatBufferBuilder builder;
    auto rootOffset = TestTable::Pack(builder, &topLevel);
    builder.Finish(rootOffset);

    const TestTable* table = flatbuffers::GetRoot<TestTable>(builder.GetBufferPointer());
    // Set a breakpoint here and inspect in natvis
    std::ignore = std::ignore;
}
}
