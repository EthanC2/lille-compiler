/*
 * idtable.cpp
 *
 *  Created on: Jun 18, 2020
 *      Author: Michael Oudshoorn
 */



#include <iostream>
#include <string>

#include "token.h"
#include "error_handler.h"
#include "id_table.h"

using namespace std;


id_table::id_table(error_handler* err)
{
    error = err;
    scope_level = 0;
}


void id_table::dump_id_table(bool dump_all)
{
	if (!dump_all)
	{
		cout << "Dump of idtable for current scope only." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
        // INSERT CODE HERE
	}
	else
	{
		cout << "Dump of the entire symbol table." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
        // INSERT CODE HERE
	}
}

bool id_table::insert_entry(const IdTableEntry &entry)
{
    bool inserted = scopes[scope_level].insert(entry);

    if (debugging)
    {
	if (inserted)
	{
	    std::cout << "[ID TABLE] inserted \"" << entry.identifier->get_identifier_value() << "\", scope level = " << scope_level << '\n';
	}
	else
	{
	    std::cout << "[ID TABLE] FAILED inserting \"" << entry.identifier->get_identifier_value() << "\", scope level = " << scope_level << '\n';
	}
    }

    return inserted;
}

IdTableEntry* id_table::get_entry(token *identifier)
{
    IdTableEntry *entry = nullptr;
    IdTableEntry key = IdTableEntry(identifier);
    size_t scope = scope_level;

    while (scope >= 0)
    {
	// TODO: rework this to not use a temporary IdTableEntry object
	entry = scopes[scope].fetch(key);
	if (entry != nullptr)
	{
	    if (debugging)
	    {
		std::cout << "[ID TABLE] FOUND \"" << identifier->get_identifier_value()  << "\", scope = " << scope << '\n'; 
	    }
	    return entry;
	}

	--scope;
    }

    if (debugging)
    {
	std::cout << "[ID TABLE] DID NOT FIND \"" << identifier->get_identifier_value() << "\"\n";
    }
    return nullptr;
}

void id_table::enter_scope()
{
    ++scope_level;
}

void id_table::exit_scope()
{
    --scope_level;
}

size_t id_table::get_scope()
{
    return scope_level;
}
