#include <string>
#include <vector>
#include <optional>
#include <variant>
#include <cassert>
#include "scanner.h"
#include "lille_type.h"
#include "lille_kind.h"
#include "token.h"
#include "id_table.h"
#include "error_handler.h"
#include "parser.h"

#define ENTER()                                                      \
    do                                                               \
    {                                                                \
        if (this->debugging and this->verbose)                       \
        {                                                            \
            ++this->indentation;                                     \
            for (int i=0; i < this->indentation; ++i)                \
            {                                                        \
                std::cout << "    ";                                 \
            }                                                        \
                                                                     \
            std::cout << "[PARSER: ENTER " << __FUNCTION__ << "]: " << this->scan->this_token()->get_symbol()->symtostr() << '\n';  \
        }                                                            \
    } while (0)                                                      \

#define LEAVE()                                                      \
    do                                                               \
    {                                                                \
        if (this->debugging and this->verbose)                       \
        {                                                            \
            for (int i=0; i < this->indentation; ++i)                \
            {                                                        \
                std::cout << "    ";                                 \
            }                                                        \
                                                                     \
            std::cout << "[PARSER: LEAVE " << __FUNCTION__ << "]: " << this->scan->this_token()->get_symbol()->symtostr() << '\n';  \
            --this->indentation;                                     \
        }                                                            \
    } while (0) \

#define DEBUG(msg)                                                           \
    do                                                                       \
    {                                                                        \
        if (this->debugging)                                                 \
        {                                                                    \
            for (int i=0; i < this->indentation; ++i)                        \
            {                                                                \
                std::cout << "    ";					     \
            }                                                                \
            std::cout << "[PARSER: " << __FUNCTION__ << "] " << msg << '\n'; \
        }                                                                    \
    } while (0)                                                              \

parser::parser(scanner *scan_, id_table *id_tab_, error_handler *err_)
{
    assert(scan_ != nullptr);
    assert(id_tab_ != nullptr);
    assert(err_ != nullptr);

    this->indentation = -1;

    this->scan = scan_;
    this->id_tab = id_tab_;
    this->err = err_;
}

void parser::dump_tokenstream()
{
    token *tok;

    do
    {
        tok = scan->get_token();
        tok->print_token();
    } while (tok->get_sym() != symbol::end_of_program);
}

// <prog> ::= program <ident> is <block> ;
// 
// NOTE:  Although the BNF definitions do not state that there
//        is an end_of_program symbol at the end of the <prog> symbol,
//        it is implied.
void parser::prog()
{
    std::string program_name;

    ENTER();
    scan->get_token();
    id_tab->predefine_function("REAL2INT", lille_type::type_real, lille_type::type_integer);
    id_tab->predefine_function("INT2REAL", lille_type::type_integer, lille_type::type_real);
    id_tab->predefine_function("INT2STRING", lille_type::type_integer, lille_type::type_string);
    id_tab->predefine_function("REAL2STRING", lille_type::type_real, lille_type::type_string);

    if (debugging)
    {
	id_tab->dump_id_table();
    }

    DEBUG("program symbol");
    scan->must_be(symbol::program_sym);

    DEBUG("identifier symbol");
    if (this->scan->have(symbol::identifier))
    {
	program_name = this->scan->this_token()->get_identifier_value();
    }
    scan->must_be(symbol::identifier);

    DEBUG("is symbol");
    scan->must_be(symbol::is_sym);

    this->block(program_name, true);
    
    DEBUG("semicolon symbol");
    scan->must_be(symbol::semicolon_sym);
    
    if (this->scan->have(symbol::end_of_program))
    {
        DEBUG("end_of_program symbol");
        scan->must_be(symbol::end_of_program);
    }
    else
    {
        this->err->flag(this->scan->this_token(), 77);
    }

    LEAVE();    
}


// <block> ::= { <declaration> }* begin <statement_list> end [ <ident> ]
void parser::block(const std::string &name, bool is_program_identifier)
{
    ENTER();
    id_tab->enter_scope();
    
    while (in_first_of_declaration(this->scan->this_token()->get_sym()))
    {
        this->declaration();
    }

    DEBUG("begin symbol");
    scan->must_be(symbol::begin_sym);
    
    this->statement_list();
    
    DEBUG("end symbol");
    scan->must_be(symbol::end_sym);
    
    if (this->scan->have(symbol::identifier))
    {
        DEBUG("identifier symbol");
	if (scan->this_token()->get_identifier_value() != name)
	{
	    if (is_program_identifier)
	    {
		this->err->flag(this->scan->this_token(), 75);
	    }
	    else
	    {
		this->err->flag(this->scan->this_token(), 107);
	    }
	}

        scan->must_be(symbol::identifier);
    }

    LEAVE();
    id_tab->exit_scope();
}


// <declaration> ::= <ident_list> : [ constant ] <type> [:=<number> | :=<string> | :=<bool>] ; 
//                 | procedure <ident> [ ( <param_list> ) ] is <block> ; 
//                 | function <ident> [(<param_list>)] return <type> is <block> ;
void parser::declaration()
{
    ENTER();

    if (this->scan->have(symbol::identifier))
    {
        std::vector<token*> identifiers = this->ident_list();
        lille_kind kind = lille_kind(lille_kind::variable);
        lille_type type = lille_type(lille_type::type_unknown);
        std::variant<int,float,bool,std::string> value;
        bool constant = false;
            
            DEBUG("colon symbol");
            this->scan->must_be(symbol::colon_sym);

            if (this->scan->have(symbol::constant_sym))
            {
            constant = true;
            kind = lille_kind(lille_kind::constant);

                DEBUG("constant symbol");
                this->scan->must_be(symbol::constant_sym);
            }

            type = this->type();

            if (constant)
            {
            if (this->scan->have(symbol::becomes_sym))
            {
            DEBUG("becomes symbol");
            this->scan->must_be(symbol::becomes_sym);

            if (type.is_type(lille_type::type_integer))
            {
                DEBUG("<integer>");
                value = this->scan->this_token()->get_integer_value();
                this->scan->must_be(symbol::integer);
            }
            else if (type.is_type(lille_type::type_real))
            {
                if (scan->have(symbol::integer))
                {
                DEBUG("<integer>");
                value = this->scan->this_token()->get_integer_value();
                this->scan->must_be(symbol::integer);
                }
                else if (scan->have(symbol::real_num))
                {
                DEBUG("<real>");
                value = this->scan->this_token()->get_real_value();
                this->scan->must_be(symbol::real_num);
                }
                else
                {
                err->flag(scan->this_token(), 118);
                }
            }
            else if(type.is_type(lille_type::type_string))
            {
                DEBUG("<string>");
                value = this->scan->this_token()->get_string_value();
                this->scan->must_be(symbol::strng);
            }
            else if (type.is_type(lille_type::type_boolean))
            {
                DEBUG("<boolean>");
                if (this->scan->have(symbol::true_sym))
                {
                value = true;
                }
                else
                {
                value = false;
                }

                this->scan->get_token();
            }
            else
            {
                this->err->flag(this->scan->this_token(), 84);
            }
            }
            else
            {
            this->err->flag(this->scan->this_token(), 110);
            }
        }

	for (token *variable : identifiers)
	{
	    if (debugging)
	    {
		std::cout << "[ID TABLE] creating entry \"" << variable->get_identifier_value() << "\"\n";
	    }

	    id_table_entry *id = id_tab->enter_id(variable, type, kind, id_tab->get_scope(), 0, lille_type::type_unknown);

	    if (constant)
	    {
		if (verbose)
		{
		    std::cout << "[ID TABLE] setting constant value for entry \"" << variable->get_identifier_value() <<  "\"\n";
		}
	    
		id->fix_const(value);
	    }

	    if (debugging)
	    {
		std::cout << "[ID TABLE] adding entry \"" << variable->get_identifier_value() << "\" to the ID table\n";
	    }

	    id_tab->add_table_entry(id);
	}

        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);
    }
    else if (this->scan->have(symbol::procedure_sym))
    {
	id_table_entry *procedure_id = nullptr;
	token *procedure = nullptr;
	lille_kind kind = lille_kind::unknown;
	lille_type type = lille_type::type_proc;
	int level = id_tab->get_scope();
	lille_type return_type = lille_type::type_unknown;

        DEBUG("procedure symbol");
        this->scan->must_be(symbol::procedure_sym);

        DEBUG("identifier symbol");
	if (this->scan->have(symbol::identifier))
	{
	    procedure = this->scan->this_token();
	    procedure_id = id_tab->enter_id(procedure, type, kind, level, 0, return_type);
	    id_tab->add_table_entry(procedure_id);

	    if (debugging)
	    {
		std::cout << "[ID TABLE] added \"" << procedure->get_identifier_value() << "\" to scope " << id_tab->get_scope() << '\n';
	    }
	}
        this->scan->must_be(symbol::identifier);

	    id_tab->enter_scope();
        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->param_list(procedure_id);

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }

        DEBUG("is symbol");
        this->scan->must_be(symbol::is_sym);

        this->block(procedure->get_identifier_value());

        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);

	id_tab->exit_scope();
    }
    else if (this->scan->have(symbol::function_sym))
    {
	id_table_entry *function_id = nullptr;
	token *function = nullptr;
	lille_kind kind = lille_kind::unknown;
	lille_type type = lille_type::type_func;
	int level = id_tab->get_scope();
	lille_type return_type = lille_type::type_unknown;

        DEBUG("function symbol");
        this->scan->must_be(symbol::function_sym);

        DEBUG("identifier symbol");
	if (this->scan->have(symbol::identifier))
	{
	    function = this->scan->this_token();
	    function_id = id_tab->enter_id(function, type, kind, level, 0, return_type);
	    id_tab->add_table_entry(function_id);

	    if (debugging)
	    {
		std::cout << "[ID TABLE] added \"" << function->get_identifier_value() << "\" to scope " << std::to_string(id_tab->get_scope()) << '\n';
	    }
	}
        this->scan->must_be(symbol::identifier);

	    id_tab->enter_scope();
        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->param_list(function_id);

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }

        DEBUG("return symbol");
        this->scan->must_be(symbol::return_sym);

        function_id->fix_return_type(this->type());
	if (debugging)
	{
	    std::cout << "[ID TABLE: declaration::function] set type to " << function_id->get_type().to_string() << '\n';
	}

        DEBUG("is symbol");
        this->scan->must_be(symbol::is_sym);

        this->block(function_id->get_name());
        
        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);

	id_tab->exit_scope();
    }
    else
    {
        this->err->flag(this->scan->this_token(), 106);
    }

    LEAVE();
}

// <type> ::= integer | real | string | boolean
lille_type parser::type()
{
    ENTER();

    if (this->scan->have(symbol::integer_sym))
    {
	DEBUG("keyword integer");
	this->scan->must_be(symbol::integer_sym);
	return lille_type::type_integer;
    }
    else if (this->scan->have(symbol::real_sym))
    {
	DEBUG("<keyword real>");
	this->scan->must_be(symbol::real_sym);
	return lille_type::type_real;
    }
    else if (this->scan->have(symbol::string_sym))
    {
	DEBUG("<keyword string>");
	this->scan->must_be(symbol::string_sym);
	return lille_type::type_string;
    }
    else if (this->scan->have(symbol::boolean_sym))
    {
	DEBUG("<keyword boolean>");
	this->scan->must_be(symbol::boolean_sym);
	return lille_type::type_boolean;
    }
    else
    {
        this->err->flag(scan->this_token(), 96);
	return lille_type::type_unknown;
    }

    LEAVE();
}

// <param_list> ::= <param> { ; <param> }*
void parser::param_list(id_table_entry *subprogram)
{
    ENTER();

    this->param(subprogram);

    while (this->scan->have(symbol::semicolon_sym))
    {
        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);

	    this->param(subprogram);
    }

    if (not this->scan->have(symbol::right_paren_sym))
    {
        this->err->flag(this->scan->this_token(), 95);
    }

    LEAVE();
}

// <param> ::= <ident_list> : <param_kind> <type>
void parser::param(id_table_entry *subprogram)
{
    std::vector<token*> identifiers;
    id_table_entry *parameter_id = nullptr;
    lille_type type = lille_type::type_unknown;
    lille_kind kind = lille_kind::unknown;
    int level = id_tab->get_scope();

    ENTER();

    identifiers = this->ident_list();

    DEBUG("colon symbol");
    this->scan->must_be(symbol::colon_sym);

    kind = this->param_kind();

    type = this->type();

    for (token *identifier : identifiers)
    {
	parameter_id = id_tab->enter_id(identifier, type, kind, level);
	subprogram->add_parameter(parameter_id);
	id_tab->add_table_entry(parameter_id);

	if (subprogram->get_type().is_type(lille_type::type_func) and parameter_id->get_kind().is_kind(lille_kind::ref_param))
	{
	    this->err->flag(identifier, 123);
	}

	if (debugging)
	{
	    std::cout << "[ID TABLE: param] added " << parameter_id->get_name() << " as parameter to subprogram " << subprogram->get_name() << '\n';
	}
    }

    LEAVE();
    if (debugging)
    {
	std::cout << "[ID TABLE: param] found " << identifiers.size() << "  identifiers\n"; 
	std::cout << "[ID TABLE: param] current parameter count: " << subprogram->get_number_of_parameters() << '\n';
    }
}


// <ident_list> ::= <ident> { , <ident> }*
std::vector<token*> parser::ident_list()
{
    std::vector<token*> identifiers;
    ENTER();

    DEBUG("identifier symbol");
    if (this->scan->have(symbol::identifier))
    {
	identifiers.push_back(this->scan->this_token());	
    }
    this->scan->must_be(symbol::identifier);

    while (this->scan->have(symbol::comma_sym))
    {
        DEBUG("comma symbol");
        this->scan->must_be(symbol::comma_sym);

        DEBUG("identifier symbol");
	if (this->scan->have(symbol::identifier))
	{
	    identifiers.push_back(this->scan->this_token());	
	}
	this->scan->must_be(symbol::identifier);
    }

    if (debugging)
    {
	std::cout << "[ID TABLE] parsed " << identifiers.size() << " identifiers\n";
    }

    LEAVE();
    return identifiers;
}

// <param_kind> ::= value | ref
lille_kind parser::param_kind()
{
    if (this->scan->have(symbol::value_sym))
    {
        DEBUG("value");
        this->scan->must_be(symbol::value_sym);
	return lille_kind::value_param;
    }
    else if (this->scan->have(symbol::ref_sym))
    {
	DEBUG("ref");
	this->scan->must_be(symbol::ref_sym);
	return lille_kind::ref_param;
    }
    else
    {
        this->err->flag(this->scan->this_token(), 94);
	return lille_kind::unknown;
    }
}

// <statement_list> ::= <statement> ; { <statement> ; }
void parser::statement_list()
{
    ENTER();

    this->statement();

    DEBUG("semicolon symbol");
    this->scan->must_be(symbol::semicolon_sym);

    while (this->in_first_of_statement(this->scan->this_token()->get_sym()))
    {
        this->statement();
        
        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);
    }

    LEAVE();
}

// <statement> ::= <simple_statement> 
//               | <compound_statement>
void parser::statement()
{
    ENTER();

    if (this->in_first_of_simple_statement(this->scan->this_token()->get_sym()))
    {
        this->simple_statement();
    }
    else if (this->in_first_of_compound_statement(this->scan->this_token()->get_sym()))
    {
        this->compound_statement();
    }
    else
    {
        this->err->flag(this->scan->this_token(), 80);
    }

    LEAVE();
}

// <simple_statement> ::= <ident> [ ( <expr> { , <expr> }* ) ]
//                      | <ident> := <expr>
//                      | exit [ when <expr> ]
//                      | return [ <expr> ]
//                      | read [ ( ] <ident> { , <ident> }* [ ) ]
//                      | write [ ( ] <expr> { , <expr> }* [ ) ]
//                      | writeln [ ( ] [<expr> { , <expr> }* ] [ ) ]
//                      | null
void parser::simple_statement()
{
    lille_type type;
    token *tok;
    id_table_entry* entry;
    int nparam;
    ENTER();

    if (this->scan->have(symbol::identifier))
    {
        DEBUG("identifier symbol");
	    tok = scan->this_token();
	    entry = id_tab->lookup(tok->get_identifier_value());
        if (debugging)
        {
            std::cout << '[' << __FUNCTION__ << " :: identifier] entry \"" << tok->get_identifier_value() << "\" = " << entry << '\n';
        }

        if (entry == nullptr)
        {
            err->flag(tok, 81);
        }
        this->scan->must_be(symbol::identifier);

        if (this->scan->have(symbol::left_paren_sym))
        {
            if (entry != nullptr and not (entry->get_type().is_type(lille_type::type_func) or entry->get_type().is_type(lille_type::type_proc)))
            {
                err->flag(scan->this_token(), 91);
            }

                DEBUG("left parenthesis symbol");
                this->scan->must_be(symbol::left_paren_sym);

                if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
                {
                    nparam = this->expr_list(entry);

                    if (entry != nullptr and entry->get_number_of_parameters() != nparam)
                    {
                        err->flag(entry->get_token(), 98);
                    }
                }

                DEBUG("right parenthesis symbol");
                this->scan->must_be(symbol::right_paren_sym);
            }
            else if (this->scan->have(symbol::becomes_sym))
            {   
                DEBUG("becomes symbol");
                this->scan->must_be(symbol::becomes_sym);

                type = this->expr();

            if (debugging)
            {
                std::cout << "<simple_statement::assignment> rhs type: " << type.to_string() << '\n';

                if (entry == nullptr)
                {
                    std::cout << "<simple_statement::assignment> constant/variable not found\n"; 
                }
                else
                {
                    std::cout << "<simple_statement::assignment> found variable/constant \"" << entry->get_name() << "\" of type " << entry->get_type().to_string() << '\n';
                }
            }

            if (entry != nullptr and not (entry->get_kind().is_kind(lille_kind::variable) or entry->get_kind().is_kind(lille_kind::ref_param)))
            {
                err->flag(scan->this_token(), 85);
            }

            if (entry != nullptr and not entry->get_type().is_type(type))
            {
                err->flag(scan->this_token(), 93);
            }
        }
    }
    else if (this->scan->have(symbol::exit_sym))
    {
        DEBUG("exit symbol");
        this->scan->must_be(symbol::exit_sym);

        if (this->scan->have(symbol::when_sym))
        {
            DEBUG("when symbol");
            this->scan->must_be(symbol::when_sym);

            this->expr();
        }
    }
    else if (this->scan->have(symbol::return_sym))
    {
        //if (context != nullptr and not (context->get_type().is_type(lille_type::type_proc) or context->get_type().is_type(lille_type::type_func)))
        //{
        //    err->flag(scan->this_token(), 88);
        //}

        DEBUG("return symbol");
        this->scan->must_be(symbol::return_sym);

        if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
        {
            this->expr();
        }
	else
	{
	
	}
    }
    else if (this->scan->have(symbol::read_sym))
    {
        DEBUG("read symbol");
        this->scan->must_be(symbol::read_sym);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);
            
            this->ident_list();

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else
        {
            this->ident_list();
        }
    }
    else if (this->scan->have(symbol::write_sym))
    {
        DEBUG("write symbol");
        this->scan->must_be(symbol::write_sym);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->expr_list(nullptr);

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else
        {
            this->expr_list(nullptr);
        }
    }
    else if (this->scan->have(symbol::writeln_sym))
    {
        DEBUG("writeln symbol");
        this->scan->must_be(symbol::writeln_sym);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
            {
                this->expr_list(nullptr);
            }

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else
        {
            if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
            {
                this->expr_list(nullptr);
            }
        }
    }
    else if (this->scan->have(symbol::null_sym))
    {
        DEBUG("null symbol");
        this->scan->must_be(symbol::null_sym);
    }
    else
    {
        this->err->flag(this->scan->this_token(), 83);
    }

    LEAVE();
}

// <compound_statement> ::= <if_statement>
//                        | <loop_statement>
//                        | <for_statement>
//                        | <while_statement>
void parser::compound_statement()
{
    ENTER();

    if (this->scan->have(symbol::if_sym))
    {
        this->if_statement();
    }
    else if (this->scan->have(symbol::while_sym))
    {
        this->while_statement();
    }
    else if (this->scan->have(symbol::for_sym))
    {
        this->for_statement();
    }
    else if (this->scan->have(symbol::loop_sym))
    {
        this->loop_statement();
    }
    else
    {
        this->err->flag(this->scan->this_token(), 101);
    }

    LEAVE();
}

// <if_statement> ::= if <expr> then <statement_list>
//                    { elsif <expr> then <statement_list> }*
//                    [ else <statement_list> ]
//                    end if
void parser::if_statement()
{
    ENTER();

    DEBUG("if symbol");
    this->scan->must_be(symbol::if_sym);

    this->expr();

    DEBUG("then symbol");
    this->scan->must_be(symbol::then_sym);

    this->statement_list();

    while (this->scan->have(symbol::elsif_sym))
    {
        DEBUG("elsif symbol");
        this->scan->must_be(symbol::elsif_sym);

        this->expr();


        DEBUG("then symbol");
        this->scan->must_be(symbol::then_sym);

        this->statement_list();
    }

    if (this->scan->have(symbol::else_sym))
    {
        DEBUG("else symbol");
        this->scan->must_be(symbol::else_sym);

        this->statement_list();
    }

    DEBUG("end symbol");
    this->scan->must_be(symbol::end_sym);

    DEBUG("if symbol");
    this->scan->must_be(symbol::if_sym);

    LEAVE();
}

// <while_statement> ::= while <expr> <loop_statement>
void parser::while_statement()
{
    ENTER();

    DEBUG("while symbol");
    this->scan->must_be(symbol::while_sym);

    this->expr();

    this->loop_statement();

    LEAVE();
}

// <for_statement> ::= for <ident> in [ reverse ] <range> <loop_statement>
void parser::for_statement()
{
    ENTER();
    id_tab->enter_scope();

    DEBUG("for symbol");
    this->scan->must_be(symbol::for_sym);

    DEBUG("identifier symbol");
    if (scan->have(symbol::identifier))
    {
	token *identifier = scan->this_token();
	id_table_entry *entry = id_tab->enter_id(identifier, lille_type::type_integer, lille_kind::for_ident, id_tab->get_scope());
	id_tab->add_table_entry(entry);
    }
    this->scan->must_be(symbol::identifier);

    DEBUG("in symbol");
    this->scan->must_be(symbol::in_sym);

    if (this->scan->have(symbol::reverse_sym))
    {
        DEBUG("reverse symbol");
        this->scan->must_be(symbol::reverse_sym);
    }

    this->range();

    this->loop_statement();

    LEAVE();
    id_tab->exit_scope();
}

// <loop_statement> ::= loop <statement_list> end loop
void parser::loop_statement()
{
    ENTER();

    DEBUG("loop symbol");
    this->scan->must_be(symbol::loop_sym);

    this->statement_list();
    
    DEBUG("end symbol");
    this->scan->must_be(symbol::end_sym);

    DEBUG("loop symbol");
    this->scan->must_be(symbol::loop_sym);

    LEAVE();
}

// <range> ::= <simple_expr> .. <simple_expr>
void parser::range()
{
    ENTER();

    this->simple_expr();

    DEBUG("range symbol");
    this->scan->must_be(symbol::range_sym);

    this->simple_expr();

    LEAVE();
}

// <expr> ::= <simple_expr> [ <relop> <simple_expr> ]
//          | <simple_expr> in <range>
//
// <simple_expr> : ARITH <relop> <simple_expr> : ARITH => BOOLEAN
// <simple_expr> : BOOLEAN <relop> <simple_expr> : BOOLEAN => BOOLEAN
// <simple_expr> : INT in <simple_expr> : INT .. <simple_expr> : INT => BOOLEAN
lille_type parser::expr()
{
    lille_type type_lhs, type_rhs;
    ENTER();

    type_lhs = this->simple_expr();
    must_be_type(scan->this_token(), type_lhs, lille_type::type_ordered);

    if (this->is_relop(this->scan->this_token()->get_sym()))
    {
        DEBUG("<relop>");
        this->scan->get_token();

        type_rhs = this->simple_expr();
	must_be_type(scan->this_token(), type_lhs, lille_type::type_arith_or_bool);
	must_be_type(scan->this_token(), type_rhs, lille_type::type_arith_or_bool);

	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << "> lhs type: " << type_lhs.to_string() << '\n';
	    std::cout << '<' << __FUNCTION__ << "> rhs type: " << type_rhs.to_string() << '\n';
	}

	return lille_type::type_boolean;
    }
    else if (this->scan->have(symbol::in_sym))
    {
	must_be_type(scan->this_token(), type_lhs, lille_type::type_integer);
	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << "<int> in <range>> lhs type: " << type_lhs.to_string() << '\n';
	}

        DEBUG("in symbol");
        this->scan->must_be(symbol::in_sym);

        this->range();

	return lille_type::type_boolean;
    }

    LEAVE();
    return type_lhs;
}

// <expr_list> ::= <expr>{ , <expr> }*
//
// NOTE: This is NOT a part of the original grammar.
//       I have added it as an abstraction to make parsing simpler.
int parser::expr_list(id_table_entry *subprogram)
{
    ENTER();

    if (subprogram == nullptr)
    {
        int nexpr = 0;

        this->expr();
        ++nexpr;

        while (this->scan->have(symbol::comma_sym))
        {
            DEBUG("comma symbol");
            this->scan->must_be(symbol::comma_sym);

            this->expr();
            ++nexpr;
        }

        return nexpr;
    }
    else
    {
        const int nparam = subprogram->get_number_of_parameters();
        id_table_entry *formal_parameter;
        lille_type actual_parameter_type;
        bool mismatching_types = false;

        for (int i = 0; i < nparam; ++i)
        {
            formal_parameter = subprogram->get_nth_parameter(i);
            actual_parameter_type = this->expr();

            if (not formal_parameter->get_type().is_type(actual_parameter_type))
            {
                mismatching_types = true;
            }

            if (i < nparam - 1)
            {
                if (scan->have(symbol::comma_sym))
                {
                    DEBUG("comma symbol");
                    scan->must_be(symbol::comma_sym);
                }
                else
                {
                    err->flag(scan->this_token(), 97);
                }
            }
        }

        if (mismatching_types)
        {
            err->flag(scan->this_token(), 122);
        }

        return subprogram->get_number_of_parameters();
    }

    LEAVE();
}

// <simple_expr> ::= <expr2> { <stringop> <expr2> }*
//
// <expr2> : STRING { <stringop> <expr2> : STRING }+ => STRING
// <expr2> : STRING <stringop><expr2> : STRING => STRING
// <expr2> : ORDERED => ORDERED
lille_type parser::simple_expr()
{
    lille_type type_lhs, type_rhs, type_product;
    ENTER();

    type_lhs = this->expr2();
    type_product = type_lhs;
    must_be_type(scan->this_token(), type_lhs, lille_type::type_ordered);
    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << "> lhs type: " << type_lhs.to_string() << '\n';
    }

    while (this->scan->have(symbol::ampersand_sym))
    {
        DEBUG("ampersand symbol");
        this->scan->must_be(symbol::ampersand_sym);

        type_rhs = this->expr2();
	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << "> rhs type: " << type_rhs.to_string() << '\n';
	}

	type_product = compatible_type(type_product, type_rhs);
	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << "> product type: " << type_product.to_string() << '\n';
	}
    }

    LEAVE();
    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << "> product type: " << type_product.to_string() << '\n';
    }

    return type_product;
}

// <expr2> ::= <term> { { <addop> | or } <term> }*
//
// <term> : INT { <addop> <term> : INT }+ => INT
// <term> : INT { <addop> <term> : REAL }+ => REAL
// <term> : REAL { <addop> <term> : INT }+ => REAL
// <term> : REAL { <addop> <term> : REAL }+ => REAL
// <term> : BOOLEAN { or <term> : BOOLEAN }+ => BOOLEAN
// <term> : ARITH => ARITH
// <term> : ORDERED => ORDERED
lille_type parser::expr2()
{
    lille_type type_lhs, type_rhs, type_sum;
    ENTER();

    type_lhs = this->term();
    type_sum = type_lhs;
    must_be_type(scan->this_token(), type_lhs, lille_type::type_ordered);
    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << "> lhs type: " << type_lhs.to_string() << '\n'; 
    }

    while (this->is_addop(this->scan->this_token()->get_sym()) or this->scan->have(symbol::or_sym))
    {
        DEBUG("<addop> | or symbol");
        this->scan->get_token();

        type_rhs = this->term();

	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << "> rhs type: " << type_rhs.to_string() << '\n'; 
	}

	type_sum = compatible_type(type_sum, type_rhs);
    }

    LEAVE();
    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << "> sum type: " << type_sum.to_string() << '\n'; 
    }
    must_be_type(scan->this_token(), type_sum, lille_type::type_ordered);
    return type_sum;
}

// <term> ::= <factor> { { <multop> | and } <factor> }*
//
// <factor> : INT { <multop> <factor> : INT }+ => INT
// <factor> : INT { <multop> <factor> : REAL}+ => REAL
// <factor> : REAL { <multop> <factor> : INT }+ => REAL
// <factor> : REAL { <multop> <factor> : REAL }+ => REAL
// <factor> : STRING { <multop> and : STRING }+ => STRING
// <factor> : ORDERED => ORDERED
lille_type parser::term()
{
    lille_type type_lhs, type_rhs, type_product;

    type_lhs = this->factor();
    type_product = type_lhs;
    must_be_type(scan->this_token(), type_lhs, lille_type::type_ordered);

    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << " :: lhs> type: " << type_lhs.to_string() << '\n';
    }

    while (this->is_multop(this->scan->this_token()->get_sym()) or this->scan->have(symbol::and_sym))
    {
        DEBUG("<multop> | and symbol");
        this->scan->get_token();
        
        type_rhs = this->factor();

	if (debugging)
	{
	    std::cout << '<' << __FUNCTION__ << " :: rhs> type: " << type_rhs.to_string() << '\n';
	}

	type_product = compatible_type(type_product, type_rhs);
    }

    if (debugging)
    {
	std::cout << '<' << __FUNCTION__ << " :: product> type: " << type_product.to_string() << '\n';
    }

    must_be_type(scan->this_token(), type_lhs, lille_type::type_ordered);
    return type_product;
}

// <factor> ::= <primary> [ ** <primary> ]
//            | [ <addop> ] <primary>
//
// <primary> : REAL ** <primary> : INT => REAL
// <primary> : INT ** <primary> : INT => INT
// <primary> : ORDERED => ORDERED
lille_type parser::factor()
{
    lille_type type, type_power;

    if (this->is_addop(this->scan->this_token()->get_sym()))
    {
        DEBUG("<addop>");
        this->scan->get_token();

	type = this->primary();
	must_be_type(scan->this_token(), type, lille_type::type_arith);

	if (debugging)
	{
	    std::cout << "<factor :: [<addop>] <primary>>: type: " << type.to_string() << '\n';
	}

	return type;
    }
    else
    {
        type = this->primary();
	must_be_type(scan->this_token(), type, lille_type::type_ordered);

        if (this->scan->have(symbol::power_sym))
        {
	    must_be_type(scan->this_token(), type, lille_type::type_arith);

            DEBUG("power symbol");
            this->scan->must_be(symbol::power_sym);

            type_power = this->primary();
	    must_be_type(scan->this_token(), type_power, lille_type::type_integer);

	    if (debugging)
	    {
		std::cout << "<factor :: <primary> ** <primary>>: base type: " << type.to_string() << ", power type: " << type_power.to_string() << '\n';
	    }

	    return compatible_type(type, type_power);
        }

	if (debugging)
	{
	    std::cout << "<factor :: <primary>>: type: " << type.to_string() << '\n';
	}

	return type;
    }
}

// <primary> ::= not <expr>
//             | odd <expr>
//             | ( <simple_expr> )
//             | <ident> [ ( <expr>{ , <expr> }* ) ]
//             | <number>
//             | <string>
//             | <bool>
//
//  not <expr> : BOOLEAN => BOOLEAN
//  odd <expr> : INT => BOOLEAN
//  ( <simple_expr> : ORDERED ) => ORDERED
//  <ident> : LIST[ORDERED] [ ( <expr>: ORDERED { , <expr>: ORDERED }* ) ] => ORDERED
//  <number> : ARITH => ARITH
//  <string> : STRING => STRING
//  <bool> : BOOLEAN => BOOLEAN
lille_type parser::primary()
{
    lille_type type;
    int nparams = 0;

    if (this->scan->have(symbol::not_sym))
    {
        DEBUG("not symbol");
        this->scan->must_be(symbol::not_sym);
	
	type = this->expr();
	must_be_type(scan->this_token(), type, lille_type::type_boolean);

	if (debugging)
	{
	    std::cout << "<primary :: not>: " << "expr type: " << type.to_string() << ", return type: boolean\n"; 
	}

	return lille_type::type_boolean;
    }
    else if (this->scan->have(symbol::odd_sym))
    {
        DEBUG("odd symbol");
        this->scan->must_be(symbol::odd_sym);

	type = this->expr();
	must_be_type(scan->this_token(), type, lille_type::type_integer);

	if (debugging)
	{
	    std::cout << "<primary :: odd>: " << "expr type: " << type.to_string() << ", return type: boolean\n"; 
	}

	return lille_type::type_boolean;
    }
    else if (this->scan->have(symbol::left_paren_sym))
    {
        DEBUG("left parenthesis symbol");
        this->scan->must_be(symbol::left_paren_sym);

        type = this->simple_expr();
	    must_be_type(scan->this_token(), type, lille_type::type_ordered);

        DEBUG("right parenthesis symbol");
        this->scan->must_be(symbol::right_paren_sym);

	if (debugging)
	{
	    std::cout << "<primary :: simple_expr>: " << "simple_expr type: " << type.to_string() << ", return type: " << type.to_string() << '\n'; 
	}

	return type;
    }
    else if (this->scan->have(symbol::identifier))
    {
        DEBUG("identifier symbol");
        token *identifier = this->scan->this_token();
        id_table_entry *entry = id_tab->lookup(identifier->get_identifier_value());
        if (debugging)
        {
            if (entry == nullptr)
            {
            std::cout << "<primary :: identifier> DID NOT FIND entry \"" << identifier->get_identifier_value() << "\"\n";
            err->flag(scan->this_token(), 81);
            }
            else
            {
            std::cout << "<primary :: identifier> found entry \"" << entry->get_name() << "\"\n";
            }
        }
        this->scan->must_be(symbol::identifier);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            if (entry != nullptr and entry->get_number_of_parameters() > 0)
            {
                nparams = this->expr_list(entry);

                if (entry != nullptr and nparams != entry->get_number_of_parameters())
                {
                    err->flag(entry->get_token(), 97);
                }
            }

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);

            if (debugging)
            {
            if (entry == nullptr)
            {
                std::cout << "<primary :: function|procedure>: function not found. returning lille_Type::type_unknown\n";    
            }
            else
            {
                std::cout << "<primary :: function|procedure>: return type: " << entry->get_return_type().to_string() << '\n';
            }
	    }

	    return entry != nullptr ? entry->get_return_type() : lille_type::type_unknown;
	}
	else
	{
	    if (debugging)
	    {
		if (entry == nullptr)
		{
		    std::cout << "<primary :: variable|constant>: function not found. returning lille_Type::type_unknown\n";    
		}
		else
		{
		    std::cout << "<primary :: variable|constant>: type: " << entry->get_type().to_string() << '\n';
		}
	    }

	    return entry != nullptr ? entry->get_type() : lille_type::type_unknown;
	}

	return lille_type::type_unknown;
    }
    else if (this->is_number(this->scan->this_token()->get_sym()))
    {
        DEBUG("<number>");
	if (scan->have(symbol::integer))
	{
	    scan->must_be(symbol::integer);
	    if (debugging)
	    {
		std::cout << "<primary :: <number> :: integer>> type: integer\n";
	    }

	    return lille_type::type_integer;
	}
	else
	{
	    scan->must_be(symbol::real_num);
	    if (debugging)
	    {
		std::cout << "<primary :: <number> :: real> type: real number\n";
	    }

	    return lille_type::type_real;
	}
    }
    else if (this->scan->have(symbol::strng))
    {
        DEBUG("<string>");
        this->scan->must_be(symbol::strng);
	if (debugging)
	{
	    std::cout << "<primary :: <string>>: type: string\n";
	}

	return lille_type::type_string;
    }
    else if (this->is_bool(this->scan->this_token()->get_sym()))
    {
        DEBUG("<bool>");
        this->scan->get_token();
	if (debugging)
	{
	    std::cout << "<primary :: <bool>> type: boolean\n";
	}

	return lille_type::type_boolean;
    }
    else
    {
        this->err->flag(this->scan->this_token(), 84);
	if (debugging)
	{
	    std::cout << "<primary :: error>: type : unknown\n";
	}

	return lille_type::type_unknown;
    }
}

// <bool> ::= true
//          | false
bool parser::is_bool(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::true_sym)
        or this->scan->have(symbol::false_sym);
}

// <number> ::= <digit_seq> [. <digit_seq> ] [ <exp> <addop> <digit_seq>
bool parser::is_number(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::integer)
        or this->scan->have(symbol::real_num);
}

// <addop> ::= + | –
bool parser::is_addop(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::plus_sym)
        or this->scan->have(symbol::minus_sym);
}

// <multop> ::= * |
bool parser::is_multop(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::asterisk_sym)
        or this->scan->have(symbol::slash_sym);
}

// <relop> ::= > | < | = | <> | <= | >=
bool parser::is_relop(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::greater_than_sym)
        or this->scan->have(symbol::less_than_sym)
        or this->scan->have(symbol::equals_sym)
        or this->scan->have(symbol::not_equals_sym)
        or this->scan->have(symbol::greater_or_equal_sym)
        or this->scan->have(symbol::less_or_equal_sym);
}

bool parser::in_first_of_declaration(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::identifier)
        or this->scan->have(symbol::procedure_sym)
        or this->scan->have(symbol::function_sym);
}

bool parser::in_first_of_statement(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->in_first_of_simple_statement(s)
        or this->in_first_of_compound_statement(s);
}
bool parser::in_first_of_simple_statement(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->scan->have(symbol::identifier)
        or this->scan->have(symbol::exit_sym)
        or this->scan->have(symbol::return_sym)
        or this->scan->have(symbol::read_sym)
        or this->scan->have(symbol::write_sym)
        or this->scan->have(symbol::writeln_sym)
        or this->scan->have(symbol::null_sym);
}

bool parser::in_first_of_compound_statement(symbol::symbol_type s)
{
    ENTER();
    
    LEAVE();

    return this->scan->have(symbol::if_sym)
        or this->scan->have(symbol::loop_sym)
        or this->scan->have(symbol::for_sym)
        or this->scan->have(symbol::while_sym);
}

bool parser::in_first_of_expr(symbol::symbol_type s)
{
    ENTER();

    LEAVE();

    return this->is_addop(this->scan->this_token()->get_sym())
        or this->scan->have(symbol::not_sym)
        or this->scan->have(symbol::odd_sym)
        or this->scan->have(symbol::left_paren_sym)
        or this->scan->have(symbol::identifier)
        or this->is_number(this->scan->this_token()->get_sym())
        or this->scan->have(symbol::strng)
        or this->is_bool(this->scan->this_token()->get_sym());
}

// type ARITH is INT | REAL
bool parser::is_arithmetic_type(lille_type type)
{
    return type.is_type(lille_type::type_integer) or type.is_type(lille_type::type_real);
}

// type ORDERED is ARITH | STRING | BOOLEAN
bool parser::is_ordered_type(lille_type type)
{
    return is_arithmetic_type(type) or type.is_type(lille_type::type_string) or type.is_type(lille_type::type_boolean);
}

// Performs arithmetic type widening
//
// (_,REAL) => REAL
// (REAL,_) => REAL
// (_,_) => _
// _ => type::unknown
lille_type parser::compatible_type(lille_type tya, lille_type tyb)
{
    if (tya.is_type(tyb))
    {
	return tya;

	if (debugging)
	{
	    std::cout << '[' << __FUNCTION__ << "] " << tya.to_string() << " == " << tyb.to_string() << '\n';
	}
    }

    if (tya.is_type(lille_type::type_unknown) or tyb.is_type(lille_type::type_unknown))
    {
	if (debugging)
	{
	    std::cout << '[' << __FUNCTION__ << "] unknown types: " << tya.to_string() << " or " << tyb.to_string() << '\n';
	}

	return lille_type::type_unknown;
    }

    if (
	(tya.is_type(lille_type::type_real) and tyb.is_type(lille_type::type_integer)) 
     or (tya.is_type(lille_type::type_integer) and tyb.is_type(lille_type::type_real))
    )
    {
	if (debugging)
	{
	    std::cout << '[' << __FUNCTION__ << "] widened one or both to real: " << tya.to_string() << ", " << tyb.to_string() << '\n';
	}

	return lille_type::type_real;
    }

    if (debugging)
    {
	std::cout << '[' << __FUNCTION__ << "] no known category, returning lille_type::type_unknown: " << tya.to_string() << ", " << tyb.to_string() << '\n';
    }

    return lille_type::type_unknown;
}

lille_type parser::must_be_type(token *tok, lille_type tya, lille_type tyb)
{
    if (tyb.is_type(lille_type::type_ordered) and not is_ordered_type(tya))
    {
	err->flag(tok, 125);
    }
    else if (tyb.is_type(lille_type::type_arith) and not is_arithmetic_type(tya))
    {
	err->flag(tok, 116);
    }
    else if (tyb.is_type(lille_type::type_arith_or_bool) and not (is_arithmetic_type(tya) or tya.is_type(lille_type::type_boolean)))
    {
	err->flag(tok, 126);
    }
    else if (tyb.is_type(lille_type::type_arith_or_string) and not (is_arithmetic_type(tya) or tya.is_type(lille_type::type_string)))
    {
	err->flag(tok, 131);
    }
    else if (not tya.is_type(tyb))
    {
	if (tyb.is_type(lille_type::type_integer))
	{
	    err->flag(tok, 119);
	}
	else if (tyb.is_type(lille_type::type_real))
	{
	    err->flag(tok, 127);
	}
	else if (tyb.is_type(lille_type::type_string))
	{
	    err->flag(tok, 128);
	}
	else if (tyb.is_type(lille_type::type_proc))
	{
	    err->flag(tok, 129);
	}
	else if (tyb.is_type(lille_type::type_func))
	{
	    err->flag(tok, 130);
	}
	else if (tyb.is_type(lille_type::type_boolean))
	{
	    err->flag(tok, 120);
	}
	else if (tyb.is_type(lille_type::type_prog))
	{
	    err->flag(tok, 132);
	}
    }

    return tya;
}
