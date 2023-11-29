/*
 * lilletype.h
 *
 *  Created on: Jun 18, 2020
 *      Author: Michael Oudshoorn
 */

#ifndef LILLE_TYPE_H_
#define LILLE_TYPE_H_

#include <iostream>
#include <string>

#include "lille_exception.h"

using namespace std;

class lille_type {
public:
	enum lille_ty
	{
		type_integer,
		type_real,
		type_string,
		type_proc,
		type_func,
		type_boolean,
		type_unknown,
		type_arith,
		type_arith_or_string,
		type_prog,
		type_ordered,
		type_arith_or_bool
	};


	lille_type();
	lille_type(lille_ty typ);

	lille_ty get_type();
	bool is_type(lille_type typ);
	bool is_type(lille_ty typ);
	bool is_equal(lille_type typ);
	int size_of();
	string to_string();

private:
	lille_ty ty;


};

#endif /* LILLE_TYPE_H_ */
