#ifndef PARSER_H_
#define PARSER_H_

#include <vector>

#include "symbol.h"
#include "lille_type.h"
#include "token.h"
#include "scanner.h"
#include "id_table.h"
#include "error_handler.h"

using namespace std;

class parser
{
    public:
        parser(scanner *scan_, id_table *id_tab_, error_handler *err_);
        void dump_tokenstream();

        void prog();
        void block();
        void declaration();

        lille_type type();
	void param_list();
        void param();
	std::vector<token*> ident_list();        
        void param_kind();

        void statement_list();
        void statement();
        void simple_statement();
        void compound_statement();

        void if_statement();
        void while_statement();
        void for_statement();
        void loop_statement();

        void range();
        void expr();
        void expr_list();
        void simple_expr();
        void expr2();
        void term();
        void factor();
        void primary();

        bool is_number(symbol::symbol_type s);
        bool is_bool(symbol::symbol_type s);

        bool is_relop(symbol::symbol_type s);
        bool is_addop(symbol::symbol_type s);
        bool is_multop(symbol::symbol_type s);

        bool in_first_of_declaration(symbol::symbol_type s);
        bool in_first_of_statement(symbol::symbol_type s);
        bool in_first_of_simple_statement(symbol::symbol_type s);
        bool in_first_of_compound_statement(symbol::symbol_type s);
        bool in_first_of_expr(symbol::symbol_type s);

    private:
        bool debugging {false};
        bool verbose {false};
        int indentation;

        scanner *scan;
        id_table *id_tab;
        error_handler *err;
};

#endif
