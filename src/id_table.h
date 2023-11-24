/*
 * idtable.h
 *
 *  Created on: Jun 18, 2020
 *      Author: Michael Oudshoorn
 */

#ifndef ID_TABLE_H_
#define ID_TABLE_H_

#include <iostream>
#include <vector>
#include <string>

#include "token.h"
#include "error_handler.h"
#include "lille_kind.h"
#include "lille_type.h"
#include "binary_tree.h"

using namespace std;

struct IdTableEntry
{
    token *identifier;
    lille_kind kind;
    lille_type type;

    IdTableEntry(token *id, lille_kind k = lille_kind::unknown, lille_type ty = lille_type::type_unknown)
    {
	identifier = id;
	kind = k;
	type = ty;
    }

    bool operator==(const IdTableEntry &rhs)
    {
	return identifier == rhs.identifier;
    }
};

class id_table 
{
    private:
	    bool debugging {true};
	    std::vector<BinaryTree<IdTableEntry>> scopes;
	    size_t scope_level;
	    error_handler* error;

    public:
	id_table(error_handler* err);
	bool insert_entry(const IdTableEntry &entry);
	IdTableEntry* get_entry(token *identifier);
	void enter_scope();
	void exit_scope();
	size_t get_scope();
	
	void dump_id_table(bool dump_all = true);
};

#endif /* ID_TABLE_H_ */
