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
#include <memory>
#include <string>
#include <unordered_map>

#include "token.h"
#include "error_handler.h"
#include "lille_kind.h"
#include "lille_type.h"
#include "id_table_entry.h"

using namespace std;

struct IdTableNode
{
    IdTableNode(id_table_entry *entry_)
    {
	entry = entry_;
	left = nullptr;
	right = nullptr;
    }

    id_table_entry *entry;
    IdTableNode *left;
    IdTableNode *right;
};

class id_table 
{
    private:
	    error_handler* error;
	    bool debugging {false};
	    bool verbose {false};
	    std::vector<std::unordered_map<std::string,id_table_entry*>> scopes;
	    int scope_level;

	    void dump_map(const std::unordered_map<std::string,id_table_entry*>& map);
    public:
	id_table(error_handler* err);
	~id_table();
	void enter_scope();
	void exit_scope();
	size_t get_scope();
	id_table_entry* lookup(const std::string &name);
	void trace_all(bool b);
	//bool trace_all();
	bool add_table_entry(id_table_entry *id);
	void predefine_function(const std::string &name, lille_type argument_type, lille_type return_type);
	id_table_entry* enter_id(
		token *id,
		lille_type typ = lille_type::type_unknown,
		lille_kind kind = lille_kind::unknown,
		int level = 0,
		int offset = 0,
		lille_type return_tipe = lille_type::type_unknown
	);
	void dump_id_table(bool dump_all = true);
};

#endif /* ID_TABLE_H_ */
