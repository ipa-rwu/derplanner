//
// Copyright (c) 2015 Alexander Shafranov shafranov@gmail.com
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include <string.h> // memset
#include "derplanner/runtime/database.h"

using namespace plnnr;

Fact_Table plnnr::create_fact_table(Memory* mem, const Fact_Type& format, uint32_t max_entries)
{
    Fact_Table result;
    memset(&result, 0, sizeof(result));

    result.num_entries = 0;
    result.max_entries = max_entries;

    size_t size = 0;
    // data columns
    for (uint8_t i = 0; i < format.arity; ++i)
    {
        size += get_type_alignment(format.param_type[i]) + max_entries * get_type_size(format.param_type[i]);
    }
    // generations
    size = plnnr::align(size, plnnr_alignof(uint32_t));
    size += sizeof(uint32_t) * max_entries;

    void* blob = mem->allocate(size);
    memset(blob, 0, size);

    // setup pointers
    result.blob = blob;
    uint8_t* bytes = static_cast<uint8_t*>(blob);

    for (uint8_t i = 0; i < format.arity; ++i)
    {
        Type param_type = format.param_type[i];
        size_t param_align = get_type_alignment(param_type);
        uint8_t* column = static_cast<uint8_t*>(plnnr::align(bytes, param_align));
        bytes = column + max_entries * get_type_size(param_type);
        result.columns[i] = column;
    }

    result.generations = plnnr::align<uint32_t>(bytes);
    return result;
}

void plnnr::destroy(Memory* mem, Fact_Table& t)
{
    mem->deallocate(t.blob);
    memset(&t, 0, sizeof(t));
}

Fact_Database plnnr::create_fact_database(Memory* mem, const Database_Format& format)
{
    Fact_Database result;
    memset(&result, 0, sizeof(result));

    result.num_tables = format.num_tables;
    result.max_tables = format.num_tables;

    const uint32_t max_tables = format.num_tables;
    // hashes
    size_t size = plnnr_alignof(uint32_t) + sizeof(uint32_t) * max_tables;
    // names
    size = align(size, plnnr_alignof(void*));
    size += sizeof(void*) * max_tables;
    // types
    size = align(size, plnnr_alignof(Fact_Type));
    size += sizeof(Fact_Type) * max_tables;
    // tables
    size = align(size, plnnr_alignof(Fact_Table));
    size += sizeof(Fact_Table) * max_tables;

    void* blob = mem->allocate(size);
    result.blob = blob;
    uint8_t* bytes = static_cast<uint8_t*>(blob);

    result.hashes = plnnr::align<uint32_t>(bytes);
    bytes += sizeof(uint32_t) * max_tables;
    result.names = plnnr::align<const char*>(bytes);
    bytes += sizeof(void*) * max_tables;
    result.types = plnnr::align<Fact_Type>(bytes);
    bytes += sizeof(Fact_Type) * max_tables;
    result.tables = plnnr::align<Fact_Table>(bytes);

    for (uint32_t i = 0; i < format.num_tables; ++i)
    {
        uint32_t size_hint = format.size_hints[i];
        uint32_t max_entries = ( size_hint > 0 ) ? size_hint : 128;
        result.hashes[i] = format.hashes[i];
        result.names[i] = format.names[i];
        result.types[i] = format.types[i];
        Fact_Table table = create_fact_table(mem, format.types[i], max_entries);
        result.tables[i] = table;
    }

    return result;
}

void plnnr::destroy(Memory* mem, Fact_Database& db)
{
    for (uint32_t i = 0; i < db.num_tables; ++i)
    {
        Fact_Table& table = db.tables[i];
        destroy(mem, table);
    }

    mem->deallocate(db.blob);
    memset(&db, 0, sizeof(db));
}