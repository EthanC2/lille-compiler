/*
 * idtable.cpp
 *
 *  Created on: Jun 18, 2020
 *      Author: Michael Oudshoorn
 */



#include <iostream>
#include <memory>
#include <string>

#include "token.h"
#include "error_handler.h"
#include "id_table.h"
#include "id_table_entry.h"
#include "lille_exception.h"

using namespace std;


id_table::id_table(error_handler* error): scopes({{}})
{
    error = error;
    scope_level = 0;
}

id_table::~id_table()
{
    if (debugging)
    {
    	dump_id_table(true);
    } 
}

void id_table::dump_id_table(bool dump_all)
{
	if (not dump_all)
	{
		cout << "Dump of idtable for current scope only." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
		dump_map(scopes[scope_level]);
	}
	else
	{
		cout << "Dump of the entire symbol table." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
		for (int i=0; i < scopes.size(); ++i)
		{
		    std::cout << "\nSCOPE " << i << ":\n";

		    dump_map(scopes[i]);
		}
	}
}

void id_table::enter_scope()
{
    ++scope_level;

    if (scope_level >= scopes.size() - 1)
    {
	scopes.push_back({});
    }

    if (debugging)
    {
	std::cout << "[ID TABLE] incremented scope: " << scope_level << " => " << scope_level+1 << '\n';
	std::cout.put('\n');
    }
}

void id_table::exit_scope()
{
    --scope_level;

    if (debugging)
    {
	std::cout << "[ID TABLE] decremented scope: " << scope_level << " => " << scope_level-1 << '\n';
    }
}

size_t id_table::get_scope()
{
    return scope_level;
}

id_table_entry* id_table::lookup(const std::string &name)
{
    int level = scope_level;

    while (level >= 0)
    {
	if (debugging)
	{
	    std::cout << "[ID TABLE] Searching scope level " << level << " for identifier \"" << name << "\"\n";
	}

	if (scopes[level].find(name) != scopes[level].end())
	{
	    if (debugging)
	    {
		std::cout << "[ID TABLE: lookup] found \"" << name << "\" at scope level " << level << '\n';
	    }

	    return scopes[level][name];
	}

	--level;
    }

    if (debugging)
    {
	std::cout << "[ID TABLE: lookup] did not find \"" << name << "\"\n";
    }

    if (verbose)
    {
	dump_id_table();
    }

    return nullptr;
}

void id_table::trace_all(bool b)
{
    for (auto& scope : scopes)
	{
		for (auto& [name, entry] : scope)
		{
			entry->set_trace(true);
		}
	}
}


bool id_table::add_table_entry(id_table_entry *id)
{
    if (debugging)
    {
	std::cout << "[ID TABLE: " << __FUNCTION__ << "] adding table entry \"" << id->get_name() << "\" to scope level " << scope_level << '\n';
    }

    const std::string &identifier = id->get_name();
    
    if (verbose)
    {
	std::cout << "[ID TABLE] got identifier name: \"" << identifier << "\"\n";
    }

    scopes[scope_level][identifier] = id;

    if (verbose)
    {
	std::cout << "[ID TABLE] assigned identifier \"" << identifier << "\" to scope level " << scope_level << '\n'; 
    }

    return true;
}

id_table_entry* id_table::enter_id(token *id, lille_type typ, lille_kind kind, int level, int offset, lille_type return_tipe)
{
    return new id_table_entry(id, typ, kind, level, offset, return_tipe);
}

// All predefined functions have one value parameter, so no need for variadic templates
void id_table::predefine_function(const std::string &name, lille_type argument_type, lille_type return_type)
{
    if (debugging)
    {
	std::cout << "[ID TABLE: PREDEFINE FUNCTION] predefining function \"" << name << "\" with parameter type " << argument_type.to_string() << " and return type " << return_type.to_string() << '\n';
    }

    token *predefined_func;
    token *argument;
    symbol *predefined_sym;
    id_table_entry *func_id;
    id_table_entry *param_id;

    // Define predefined function
    predefined_sym = new symbol(symbol::identifier);
    predefined_func = new token(predefined_sym, 0, 0);
    predefined_func->set_identifier_value(name);
    func_id = enter_id(predefined_func, lille_type::type_func, lille_kind::unknown, /* scope = */ 0, 0, return_type);

    // Define predefined function argument
    argument = new token(predefined_sym, 0, 0);
    argument->set_identifier_value(name + "_arg__");

    param_id = new id_table_entry(
	argument,
	argument_type,
	lille_kind::value_param,
	/* scope = */ 0,
	0,
	lille_type::type_unknown
    );

    func_id->add_parameter(param_id);

    // Add predefined function to table
    add_table_entry(func_id);
    add_table_entry(param_id);
}

void id_table::dump_map(const std::unordered_map<std::string,id_table_entry*>& map)
{
    for (const auto& [name, entry] : map)
    {
	std::cout << name << " = " << entry->to_string() << '\n';
    }
}
