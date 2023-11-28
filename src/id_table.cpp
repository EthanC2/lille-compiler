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


id_table::id_table(error_handler* error): scopes({nullptr})
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
		
		dump_tree(scopes[scope_level]);
	}
	else
	{
		cout << "Dump of the entire symbol table." << endl;
		cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
		
		for (int i=0; i < scopes.size(); ++i)
		{
		    std::cout << "\nSCOPE " << i << ":\n";

		    if (scopes[i] == nullptr)
		    {
			std::cout << "<EMPTY>\n";
		    }
		    else
		    {
			dump_tree(scopes[i]);
		    }
		}
	}
}

void id_table::enter_scope()
{
    ++scope_level;

    if (scope_level >= scopes.size() - 1)
    {
	scopes.push_back(nullptr);
    }

    if (debugging)
    {
	std::cout << "[ID TABLE] incremented scope: " << scope_level << " => " << scope_level+1 << '\n';
	std::cout << "[ID TABLE] scopes: ";
	for (const IdTableNode* scope : scopes)
	{
	    if (scope == nullptr)
	    {
		std::cout << "nullptr";
	    }
	    else
	    {
		std::cout << scope;
	    }

	    std::cout << ", ";
	}
	std::cout.put('\n');
    }
}

void id_table::exit_scope()
{
    --scope_level;

    if (debugging)
    {
	std::cout << "[ID TABLE] decremented scope: " << scope_level << " => " << scope_level-1 << '\n';
	for (const IdTableNode* scope : scopes)
	{
	    std::cout << scope << ", ";
	}
	std::cout.put('\n');
    }
}

size_t id_table::get_scope()
{
    return scope_level;
}

id_table_entry* id_table::lookup(const std::string &name)
{
    int level = scope_level;
    id_table_entry *entry = nullptr;

    while (level >= 0)
    {
	if (debugging)
	{
	    std::cout << "[ID TABLE] Searching scope level " << level << " for identifier \"" << name << "\"\n";
	}
	entry = lookup(scopes[level], name);

	if (entry != nullptr)
	{
	    if (debugging)
	    {
		std::cout << "[ID TABLE] found identifer \"" << name << "\" at scope level " << level << '\n';
	    }

	    return entry;
	}
	else if (debugging)
	{
	   std::cout << "[ID TABLE] did NOT identifer \"" << name << "\" at scope level " << level << '\n';
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

id_table_entry* id_table::lookup(IdTableNode *node, const std::string &name)
{
    if (node == nullptr)
    {
	return nullptr;
    }

    if (name < node->entry->get_name())
    {
	return lookup(node->left, name);
    }
    else if (name > node->entry->get_name())
    {
	return lookup(node->right, name);
    }
    else
    {
	return node->entry;
    }
}

id_table_entry* id_table::lookup(token *tok)
{
    int level = scope_level;
    id_table_entry *entry = nullptr;

    while (level >= 0)
    {
	entry = lookup(scopes[level], tok);
	if (entry != nullptr)
	{
	    return entry;
	}

	--level;
    }

    error->flag(tok, 81);
    return nullptr;
}

id_table_entry* id_table::lookup(IdTableNode *node, token *tok)
{
    if (node == nullptr)
    {
	return nullptr;
    }

    if (tok < node->entry->get_token())
    {
	return lookup(node->left, tok);
    }
    else if (tok > node->entry->get_token())
    {
	return lookup(node->right, tok);
    }
    else
    {
	return node->entry;
    }
}

void id_table::trace_all(bool b)
{
    // TODO
    //for each variable in each scope
    //	    variable.set_trace(true);
}


bool id_table::add_table_entry(id_table_entry *id)
{
    if (debugging)
    {
	std::cout << "[ID TABLE: " << __FUNCTION__ << "] adding table entry \"" << id->get_name() << "\" to scope level " << scope_level << '\n';
    }

    return add_table_entry(scopes[scope_level], id);
}

bool id_table::add_table_entry(IdTableNode *node, id_table_entry *entry)
{
    if (/*node == scopes[scope_level] &&*/ scopes[scope_level] == nullptr)
    {
	if (debugging)
	{
	    std::cout << "[ID TABLE: add_table_entry] added \"" << entry->get_name() << "\" as root node at scope " << scope_level << '\n';
	}

	scopes[scope_level] = new IdTableNode(entry);
	return true;
    }

    if (entry < node->entry)
    {
	if (node->left == nullptr)
	{
	    node->left = new IdTableNode(entry);
	    return true;
	}
	else
	{
	    return add_table_entry(node->left, entry);
	}
    }
    else if (entry > node->entry)
    {
	if (node->right == nullptr)
	{
	    node->right = new IdTableNode(entry);
	    return true;
	}
	else
	{
	    return add_table_entry(node->right, entry);
	}
    }
    else
    {
	if (debugging)
	{
	    std::cout << "[ID TABLE] DUPLICATE ENTRY: \"" << entry->get_name() << "\" in scope " << std::to_string(scope_level) << '\n';
	}
	
	error->flag(entry->get_token(), 82);
	
	return false;
    }
}

id_table_entry* id_table::enter_id(token *id, lille_type typ, lille_kind kind, int level, int offset, lille_type return_tipe)
{
    return new id_table_entry(id, typ, kind, level, offset, return_tipe);
}

void id_table::dump_tree(IdTableNode *node, int depth)
{
    if (node == nullptr)
    {
	return;
    }

    std::cout << "[depth: " << std::to_string(depth) << "] " << node->entry->to_string() << '\n';

    dump_tree(node->left, depth + 1);
    dump_tree(node->right, depth + 1);
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
