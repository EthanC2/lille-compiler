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
            std::cout << "[DEBUG: ENTER " << __FUNCTION__ << "]: " << this->scan->this_token()->get_symbol()->symtostr() << '\n';  \
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
            std::cout << "[DEBUG: LEAVE " << __FUNCTION__ << "]: " << this->scan->this_token()->get_symbol()->symtostr() << '\n';  \
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
                std::cout << "    ";                                           \
            }                                                                \
            std::cout << "[DEBUG: " << __FUNCTION__ << "] " << msg << '\n';  \
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
    ENTER();
    scan->get_token();

    DEBUG("program symbol");
    scan->must_be(symbol::program_sym);

    DEBUG("identifier symbol");
    scan->must_be(symbol::identifier);

    DEBUG("is symbol");
    scan->must_be(symbol::is_sym);

    this->block();
    
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
void parser::block()
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

		if (this->scan->have(symbol::integer))
		{
		    DEBUG("<integer>");
		    value = this->scan->this_token()->get_integer_value();
		    if (type.get_type() != lille_type::type_integer)
		    {
			this->err->flag(this->scan->this_token(), 111);
		    }

		    this->scan->must_be(symbol::integer);
		}
		else if (this->scan->have(symbol::real_num))
		{
		    DEBUG("<real>");
		    value = this->scan->this_token()->get_real_value();
		    if (type.get_type() != lille_type::type_real)
		    {
			this->err->flag(this->scan->this_token(), 111);
		    }

		    this->scan->must_be(symbol::real_num);
		}
		else if(this->scan->have(symbol::strng))
		{
		    DEBUG("<string>");
		    value = this->scan->this_token()->get_string_value();
		    if (type.get_type() != lille_type::type_string)
		    {
			this->err->flag(this->scan->this_token(), 111);
		    }

		    this->scan->must_be(symbol::strng);
		}
		else if (this->scan->have(symbol::true_sym) or this->scan->have(symbol::false_sym))
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

		    if (type.get_type() != lille_type::type_boolean)
		    {
			this->err->flag(this->scan->this_token(), 111);
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
        DEBUG("procedure symbol");
        this->scan->must_be(symbol::procedure_sym);

        DEBUG("identifier symbol");
        this->scan->must_be(symbol::identifier);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->param_list();

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }

        DEBUG("is symbol");
        this->scan->must_be(symbol::is_sym);

        this->block();

        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);
    }
    else if (this->scan->have(symbol::function_sym))
    {
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
	}
        this->scan->must_be(symbol::identifier);

	id_tab->enter_scope();
        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->param_list();

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }

        DEBUG("return symbol");
        this->scan->must_be(symbol::return_sym);

        return_type = this->type();

        DEBUG("is symbol");
        this->scan->must_be(symbol::is_sym);

        this->block();
        
        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);

	id_table_entry *function_id = id_tab->enter_id(function, type, kind, level, 0, return_type);
	id_tab->add_table_entry(function_id);

	if (debugging)
	{
	    std::cout << "[ID TABLE] added \"" << function->get_identifier_value() << "\" to scope " << std::to_string(id_tab->get_scope()) << '\n';
	}

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
void parser::param_list()
{
    ENTER();

    this->param();

    while (this->scan->have(symbol::semicolon_sym))
    {
        DEBUG("semicolon symbol");
        this->scan->must_be(symbol::semicolon_sym);

        this->param();
    }

    if (not this->scan->have(symbol::right_paren_sym))
    {
        this->err->flag(this->scan->this_token(), 95);
    }

    LEAVE();
}

// <param> ::= <ident_list> : <param_kind> <type>
void parser::param()
{
    ENTER();

    this->ident_list();

    DEBUG("colon symbol");
    this->scan->must_be(symbol::colon_sym);

    this->param_kind();

    this->type();

    LEAVE();
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
void parser::param_kind()
{
    if (this->scan->have(symbol::value_sym) or this->scan->have(symbol::ref_sym))
    {
        DEBUG("value | ref");
        this->scan->get_token();
    }
    else
    {
        this->err->flag(this->scan->this_token(), 94);
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
    ENTER();

    if (this->scan->have(symbol::identifier))
    {
        DEBUG("identifier symbol");
        this->scan->must_be(symbol::identifier);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
            {
                this->expr_list();
            }

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else if (this->scan->have(symbol::becomes_sym))
        {
            DEBUG("becomes symbol");
            this->scan->must_be(symbol::becomes_sym);

            this->expr();
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
        DEBUG("return symbol");
        this->scan->must_be(symbol::return_sym);

        if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
        {
            this->expr();
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

            this->expr_list();

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else
        {
            this->expr_list();
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
                this->expr_list();
            }

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
        else
        {
            if (this->in_first_of_expr(this->scan->this_token()->get_sym()))
            {
                this->expr_list();
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
void parser::expr()
{
    ENTER();

    this->simple_expr();

    if (this->is_relop(this->scan->this_token()->get_sym()))
    {
        DEBUG("<relop>");
        this->scan->get_token();

        this->simple_expr();
    }
    else if (this->scan->have(symbol::in_sym))
    {
        DEBUG("in symbol");
        this->scan->must_be(symbol::in_sym);

        this->range();
    }

    LEAVE();
}

// <expr_list> ::= <expr>{ , <expr> }*
//
// NOTE: This is NOT a part of the original grammar.
//       I have added it as an abstraction to make parsing simpler.
void parser::expr_list()
{
    ENTER();

    this->expr();

    while (this->scan->have(symbol::comma_sym))
    {
        DEBUG("comma symbol");
        this->scan->must_be(symbol::comma_sym);

        this->expr();
    }

    LEAVE();
}

// <simple_expr> ::= <expr2> { <stringop> <expr2> }*
void parser::simple_expr()
{
    ENTER();

    this->expr2();

    while (this->scan->have(symbol::ampersand_sym))
    {
        DEBUG("ampersand symbol");
        this->scan->must_be(symbol::ampersand_sym);

        this->expr2();
    }

    LEAVE();
}

// <expr2> ::= <term> { { <addop> | or } <term> }*
void parser::expr2()
{
    ENTER();

    this->term();

    while (this->is_addop(this->scan->this_token()->get_sym()) or this->scan->have(symbol::or_sym))
    {
        DEBUG("<addop> | or symbol");
        this->scan->get_token();

        this->term();
    }

    LEAVE();
}

// <term> ::= <factor> { { <multop> | and } <factor> }*
void parser::term()
{
    this->factor();

    while (this->is_multop(this->scan->this_token()->get_sym()) or this->scan->have(symbol::and_sym))
    {
        DEBUG("<multop> | and symbol");
        this->scan->get_token();
        
        this->factor();
    }
}

// <factor> ::= <primary> [ ** <primary> ]
//            | [ <addop> ] <primary>
void parser::factor()
{
    if (this->is_addop(this->scan->this_token()->get_sym()))
    {
        DEBUG("<addop>");
        this->scan->get_token();

        this->primary();
    }
    else
    {
        this->primary();

        if (this->scan->have(symbol::power_sym))
        {
            DEBUG("power symbol");
            this->scan->must_be(symbol::power_sym);

            this->primary();
        }
    }
}

// <primary> ::= not <expr>
//             | odd <expr>
//             | ( <simple_expr> )
//             | <ident> [ ( <expr>{ , <expr> }* ) ]
//             | <number>
//             | <string>
//             | <bool>
void parser::primary()
{
    if (this->scan->have(symbol::not_sym))
    {
        DEBUG("not symbol");
        this->scan->must_be(symbol::not_sym);

        this->expr();
    }
    else if (this->scan->have(symbol::odd_sym))
    {
        DEBUG("odd symbol");
        this->scan->must_be(symbol::odd_sym);

        this->expr();
    }
    else if (this->scan->have(symbol::left_paren_sym))
    {
        DEBUG("left parenthesis symbol");
        this->scan->must_be(symbol::left_paren_sym);

        this->simple_expr();

        DEBUG("right parenthesis symbol");
        this->scan->must_be(symbol::right_paren_sym);
    }
    else if (this->scan->have(symbol::identifier))
    {
        DEBUG("identifier symbol");
        this->scan->must_be(symbol::identifier);

        if (this->scan->have(symbol::left_paren_sym))
        {
            DEBUG("left parenthesis symbol");
            this->scan->must_be(symbol::left_paren_sym);

            this->expr_list();

            DEBUG("right parenthesis symbol");
            this->scan->must_be(symbol::right_paren_sym);
        }
    }
    else if (this->is_number(this->scan->this_token()->get_sym()))
    {
        DEBUG("<number>");
        this->scan->get_token();
    }
    else if (this->scan->have(symbol::strng))
    {
        DEBUG("<string>");
        this->scan->must_be(symbol::strng);
    }
    else if (this->is_bool(this->scan->this_token()->get_sym()))
    {
        DEBUG("<bool>");
        this->scan->get_token();
    }
    else
    {
        this->err->flag(this->scan->this_token(), 84);
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
