#include "DebuggerTestFlatBuffers_generated.h"
#include <gtest/gtest.h>
#include <memory>

namespace Phantom::FlatBuffers::Natvis::Test
{
using namespace FlatBuffers;

TEST(NatvisDebuggerTests, Step_into_this_and_look_at_visualization_in_VisualStudio)
{
    const TestTable* table;
    const TestTable* table2;
    flatbuffers::FlatBufferBuilder builder;
    {
        TestTableT topLevel;

        auto& subTable1 = topLevel.sub_table = std::make_unique<SubTableT>();
        subTable1->value = 1;

        auto& subTable2 = topLevel.vector_table.emplace_back(std::make_unique<SubTableT>());
        subTable2->value = 2;
        auto& subTable3 = topLevel.vector_table.emplace_back(std::make_unique<SubTableT>());
        subTable3->value = 3;

        auto& subTable2_1 = topLevel.sub_table_2 = std::make_unique<SubTable2T>();
        subTable2_1->value_2 = 99;

        topLevel.string_value = "hello world";
        topLevel.byte_vector.push_back(1);
        topLevel.byte_vector.push_back(2);
        topLevel.byte_vector.push_back(3);
        topLevel.double_vector.push_back(0.5);
        topLevel.double_vector.push_back(0.75);
        topLevel.double_vector.push_back(0.885);

        SubTable2T unionSubTable;
        unionSubTable.value_2 = 0xfe;
        topLevel.union_value.Set(unionSubTable);

        topLevel.bool_value = true;
        topLevel.byte_value = 0xf0;
        topLevel.double_value = .75;
        topLevel.float_value = 1.25;
        topLevel.int_value = 0xf6543210;
        topLevel.long_value = (0xedcba09876543210LL | (0xfLL << 60));
        topLevel.short_value = 0xf210;
        topLevel.ubyte_value = 0xf0;
        topLevel.uint_value = 0xf6543210;
        topLevel.ulong_value = (0xedcba09876543210ULL | (0xfULL << 60));
        topLevel.ushort_value = 0xf210;
        
        topLevel.struct_value = std::make_unique<TestStruct>(17);

        topLevel.string_vector.push_back("hello");
        topLevel.string_vector.push_back("world");

        topLevel.struct_vector.push_back(TestStruct(15));
        topLevel.struct_vector.push_back(TestStruct(16));

        auto rootOffset = TestTable::Pack(builder, &topLevel);
        builder.Finish(rootOffset);

        table = flatbuffers::GetRoot<TestTable>(builder.GetBufferPointer());
    }
    {
        TestTableT topLevel;
        topLevel.double_vector.push_back(0.5);
        auto rootOffset = TestTable::Pack(builder, &topLevel);
        builder.Finish(rootOffset);

        table2 = flatbuffers::GetRoot<TestTable>(builder.GetBufferPointer());
    }
    // Set a breakpoint here and inspect in natvis
    std::ignore = std::ignore;
}
}
