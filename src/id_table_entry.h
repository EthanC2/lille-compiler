#ifndef ID_TABLE_ENTRY_H
#define ID_TABLE_ENTRY_H

#include <iostream>
#include <vector>
#include <optional>
#include <variant>
#include <memory>
#include <string>

#include "token.h"
#include "lille_type.h"
#include "lille_kind.h"

class id_table_entry
{
    public:
	id_table_entry();
	id_table_entry(
	    token *id_,
	    lille_type type_ = lille_type::type_unknown,
	    lille_kind kind_ = lille_kind::unknown,
	    int level_ = 0,
	    int offset_ = 0,
	    lille_type return_type_ = lille_type::type_unknown
	);

	token* get_token();
	bool get_trace();
	void set_trace(bool boolean);
	int get_offset();
	int get_level();
	lille_kind get_kind();
	lille_type get_type();
	std::string get_name() const;
	int get_integer_value();
	float get_real_value();
	std::string get_string_value();
	lille_type get_return_type();

	void fix_const(std::variant<int,float,bool,std::string> value_);
	void fix_return_type(lille_type return_type_);
	void add_parameter(id_table_entry *parameter);
	id_table_entry* get_nth_parameter(int n);
	int get_number_of_parameters();
	std::string to_string();

	bool operator<(const id_table_entry &rhs);
	bool operator>(const id_table_entry &rhs);

    private:
	token *id;
	lille_type type;
	lille_kind kind;
	int level;
	int offset;
	bool trace;

	// Types only (type_integer || type_real || type_boolean || type_string)
	std::optional<std::variant<int,float,bool,std::string>> value;
	
	// Procedures and functions only (type_proc || type_func)
	std::optional<std::vector<id_table_entry*>> parameter_list;

	// Functions only (type_func)
	std::optional<lille_type> return_type;
};

#endif
