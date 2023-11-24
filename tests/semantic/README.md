# Types
- Type-check all expressions (variable assignments, loop conditions, ...) using Lille's type rules
    
# Type System
- Rewrite expression, term, and factor parser subroutines to return the type of the expression (integer or real). Recursive structure implicitly creates tree.
- Type information for identifiers should be stored in the identifier table

# Identifier Table
- List of sorted binary search trees, where the nth index is the nth scope
- No maximum scope depth (use std::vector<BinaryTree>)
- Identifier table stores:
    - Name (std::string, search key)
    - Kind of object (lille_kind, satellite data)
    - Type of object (lille_type, satellite data)
- Identifier table operations:
    - Constructor
    - Add identifier
    - Fetch identifier
    - Increment scope
    - Decrement scope
    - Dump table
- "Searching" the ID table starts at the current scope index and travels upwards in scope until the identifier is found or is determined to not exist (the entire table is searched)
- If an entry does not exist, it should be entered into the ID table with the type "unknown" for later error handling
- The same identifier cannot be redelcared within the same scope
- The same identifier can be redeclared in different scopes, causing the most recent one to shadow the former

# Adding the Identifier Table to the Parser
- The type of a variable cannot be known until the entire declaration has been parsed, so enter them as they are encountered and then add the types later

# Static Semantic Analysis
- Implement identifier table to store information about identifiers
- Implement a type module which checks the usage of identifiers

# Dynamic Semantic Analysis
