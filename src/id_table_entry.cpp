#include <vector>
#include <optional>
#include <variant>
#include <string>

#include "id_table_entry.h"
#include "lille_kind.h"
#include "lille_type.h"

id_table_entry::id_table_entry()
{
    id = nullptr;
    type = lille_type::type_unknown;
    lille_kind kind = lille_kind::unknown;
    level = 0;
    offset = 0;
    trace = false;
    value = std::nullopt;
    parameter_list = std::nullopt;
    return_type = std::nullopt;
}

id_table_entry::id_table_entry(token *id_, lille_type type_, lille_kind kind_, int level_, int offset_, lille_type return_type_)
{
    id = id_;
    type = type_,
    kind = kind_,
    level = level_,
    offset = offset_,
    trace = false;
    value = std::nullopt;
    parameter_list = std::nullopt;
    return_type = return_type_;
}

token* id_table_entry::get_token()
{
    return id;
}

bool id_table_entry::get_trace()
{
    return trace;
}

void id_table_entry::set_trace(bool boolean)
{
    trace = boolean;
}

int id_table_entry::get_offset()
{
    return offset;
}

int id_table_entry::get_level()
{
    return level;
}

lille_kind id_table_entry::get_kind()
{
    return kind;
}

lille_type id_table_entry::get_type()
{
    return type;
}


std::string id_table_entry::get_name() const
{
    if (id == nullptr)
    {
	throw lille_exception("cannot access name of null identifier");
    }

    return id->get_identifier_value();
}

int id_table_entry::get_integer_value()
{
    if (type.get_type() != lille_type::type_integer)
    {
	throw lille_exception("cannot access integer value of non-integer type");
    }

    return std::get<int>(*value);
}

float id_table_entry::get_real_value()
{
    if (type.get_type() != lille_type::type_real)
    {
	throw lille_exception("cannot access real value of non-real type");
    }

    return std::get<float>(*value);
}

std::string id_table_entry::get_string_value()
{
    if (type.get_type() != lille_type::type_string)
    {
	throw lille_exception("cannot access string value of non-string type");
    }

    return std::get<std::string>(*value);
}


lille_type id_table_entry::get_return_type()
{
    if (type.get_type() != lille_type::type_func)
    {
	throw lille_exception("cannot access return type of non-function type");
    }

    return *return_type;
}

void id_table_entry::fix_const(std::variant<int,float,bool,std::string> value_)
{
    value = value_;
}

void id_table_entry::fix_return_type(lille_type return_type_)
{
    if (type.get_type() != lille_type::type_func)
    {
	throw lille_exception("cannot fix return type for non-function type");
    }

    return_type = return_type_;
}

void id_table_entry::add_parameter(id_table_entry *parameter)
{
    if (type.get_type() != lille_type::type_func and type.get_type() != lille_type::type_proc)
    {
	throw lille_exception("cannot access append parameter to parameter list of non-procedure and non-function types");
    }

    if (not parameter_list.has_value())
    {
	parameter_list = {};
    }

    parameter_list->push_back(parameter);
}

id_table_entry* id_table_entry::get_nth_parameter(int n)
{
    if (type.get_type() != lille_type::type_func and type.get_type() != lille_type::type_proc)
    {
	throw lille_exception("cannot access nth parameter for non-procedure and non-function types");
    }

    if (not parameter_list.has_value())
    {
	parameter_list = {};
    }

    if (parameter_list->size() >= n)
    {
	//throw lille_exception("cannot access " + std::to_string(n) + "th parameter of a function with " + std::to_string(parameter_list->size()) + " parameters");
	return nullptr;
    }

    return (*parameter_list)[n];
}

int id_table_entry::get_number_of_parameters()
{
    if (type.get_type() != lille_type::type_func and type.get_type() != lille_type::type_proc)
    {
	throw lille_exception("cannot access number of paramters for non-procedure and non-function types");
    }

    if (not parameter_list.has_value())
    {
	parameter_list = {};
    }

    return parameter_list->size();
}

std::string id_table_entry::to_string()
{
    std::string description {"id_table_entry { name: " + get_name()
	 + ", type: " + type.to_string()
	 + ", kind: " + kind.to_string()
	 + ", level: " + std::to_string(level)
	 + ", trace: " + (trace ? "true" : "false")
    };

    if (type.get_type() == lille_type::type_proc or type.get_type() == lille_type::type_func)
    {
	if (parameter_list.has_value())
	{
	    description += "parameter list: { ";

	    for (id_table_entry *parameter : *parameter_list)
	    {
		description += parameter->to_string() + ',';
	    }

	    description += '}';
	}

	if (type.get_type() == lille_type::type_func)
	{
	    description += ", return type: " + return_type->to_string();
	}
    }

    if (kind.is_kind(lille_kind::constant))
    {
	if (type.get_type() == lille_type::type_integer)
	{
	    description += ", value: " + std::to_string(std::get<int>(*value));
	}
	else if (type.get_type() == lille_type::type_real)
	{
	    description += ", value: " + std::to_string(std::get<float>(*value));
	}
    }

    description += " }";
    return description;
}

bool id_table_entry::operator<(const id_table_entry &rhs)
{
    return this->get_name() < rhs.get_name();
}

bool id_table_entry::operator>(const id_table_entry &rhs)
{
    return this->get_name() > rhs.get_name();
}
