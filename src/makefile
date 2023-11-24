all:	compiler.o error_handler.o lille_exception.o scanner.o id_table.o symbol.o token.o parser.o id_table_entry.o lille_kind.o lille_type.o
	g++ -o compiler compiler.o error_handler.o lille_exception.o scanner.o symbol.o token.o parser.o id_table_entry.o
	echo Compilation complete.

compiler.o:	id_table.o error_handler.o lille_exception.o scanner.o symbol.o compiler.cpp
	g++ -std=c++2b -c compiler.cpp id_table.o

error_handler.o: lille_exception.o token.o error_handler.h error_handler.cpp
	g++ -std=c++2b -c error_handler.cpp

id_table.o: id_table.h id_table.cpp id_table_entry.o
	g++ -std=c++2b -c id_table.cpp

id_table_entry.o: id_table_entry.cpp id_table_entry.h lille_type.o lille_kind.o
	g++ -std=c++2b -c id_table_entry.cpp lille_kind.o lille_type.o

lille_exception.o:	lille_exception.h lille_exception.cpp
	g++ -std=c++2b -c lille_exception.cpp

scanner.o: error_handler.o lille_exception.o token.o symbol.o id_table.o scanner.h scanner.cpp
	g++ -std=c++2b -c scanner.cpp

parser.o: scanner.o id_table.o error_handler.o
	g++ -std=c++2b -c parser.cpp

symbol.o: symbol.h symbol.cpp
	g++ -std=c++2b -c symbol.cpp

token.o: lille_exception.o symbol.o token.h token.cpp
	g++ -std=c++2b -c token.cpp

lille_kind.o: lille_kind.h lille_kind.cpp
	g++ -std=c++2b -c lille_kind.cpp

lille_type.o: lille_type.h lille_type.cpp
	g++ -std=c++2b -c lille_type.cpp

clean:
	rm *.o 
	rm compiler
	echo Clean complete
