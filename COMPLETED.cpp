//
//  one.cpp
//  project1
//
//  Created by Erick Dagenais on 2/11/17.
//  Copyright © 2017 Erick Dagenais. All rights reserved.
//

/*
 * Contains the the apply function, the cofactors and quantification functions.
 *
 * For Project 1, implement
 * (1) apply, also handles probabilities
 * (2) negative_cofactor
 * (3) positive_cofactor,
 * (4) boolean_difference
 * (5) sort by influence
 */
#include <vector>
#include <set>
#include <algorithm>
#include "project1.h"
using namespace std;

class variable { //data struct to store influence with respect to its char
    public:
	float prob;
	char var;
	variable(float p, char v) : prob(p), var(v) {}
};

class influence{ //comp for sort by influence
    public:
	influence() {};
	bool operator()(variable & lhs, variable & rhs){
		return lhs.prob > rhs.prob;
	}
};


// wrapper function to allow calling with the operation, i.e apply(bdd1, bdd2, "or")
bdd_ptr apply(bdd_ptr bdd1, bdd_ptr bdd2, string o)
{
    operation dop;
    if (!dop.set_operation(o))
    {
        return 0;
    }
    
    return apply(bdd1, bdd2, dop);
}

// apply implements an arbitrary operation (specified in op) on two BDDs
// bdd_tables is used to handle the book keeping (see bdd_tables.h).
//
// apply works recursively one the idea that given an arbitrary operation $,
// and functions f and g: f $ g = a'(fa' $ ga') + a(fa $ ga),
// where a is a variable, fa' is the negative cofactor etc.
bdd_ptr apply(bdd_ptr bdd1, bdd_ptr bdd2, operation &op)
{
    // get reference to tables
    bdd_tables& tables = bdd_tables::getInstance();
    
    bdd_ptr result = tables.find_in_computed_table(op.get_operation(), bdd1, bdd2);
    if (result != 0) return result; //result found!
    
    result = op(bdd1, bdd2); //not found, perform operation
    if (result != 0) return result;
    
    char var = find_next_var(bdd1, bdd2); //find var to split on
    
    //recursive calls
    bdd_ptr neg = apply(negative_cofactor(bdd1, var), negative_cofactor(bdd2, var), op);
    bdd_ptr pos = apply(positive_cofactor(bdd1, var), positive_cofactor(bdd2, var), op);
    
    if (neg == pos) result = neg; //same tree, no node to be created
    
    else
    {
        result = tables.find_in_unique_table(var, neg, pos);
        if (result != 0) return result; //found in unique table
        else //create new node
        {
            result = tables.create_and_add_to_unique_table(var, neg, pos);
            result->probability = (neg->probability + pos->probability)/2; //add probability
        }
    }
    tables.insert_computed_table(op.get_operation(), bdd1, bdd2, result);
    return result;
}

// negative_cofactor takes the BDD pointed to by np,
// and returns the negative cofactor with respect to var.
bdd_ptr negative_cofactor(bdd_ptr np, char var)
{
    // get reference to tables
    bdd_tables& tables = bdd_tables::getInstance();
    
    //... your code goes here
    //base cases
    if (np->is_terminal()) return np; //check if terminal node first
    else if (var == np->var) return np->neg_cf; //var found, return negative cofactor
    
    bdd_ptr cf; //cofactor pointer
    bdd_ptr neg = negative_cofactor(np->neg_cf, var);
    bdd_ptr pos = negative_cofactor(np->pos_cf, var);
    
    if (neg == pos) cf = neg; //no need to create new node, same tree
    else
    {
        cf = tables.find_in_unique_table(np->var, neg, pos); //check to see if already exists
        if (cf != 0) return cf; //already exists
        else //create node
        {
            cf = tables.create_and_add_to_unique_table(np->var, neg, pos);
            cf->probability = (neg->probability + pos->probability)/2; //prob
        }
    }
    return cf;
}

// posative_cofactor takes the BDD pointed to by np,
// and returns the posative cofactor with respect to var.
bdd_ptr positive_cofactor(bdd_ptr np, char var)
{
    // get reference to tables
    bdd_tables& tables = bdd_tables::getInstance();
    
    //... your code goes here
    //base cases
    if (np->is_terminal()) return np; //check if terminal node first
    else if (var == np->var) return np->pos_cf; //var found, return positive cofactor
    
    bdd_ptr cf; //cofactor pointer
    bdd_ptr neg = positive_cofactor(np->neg_cf, var);
    bdd_ptr pos = positive_cofactor(np->pos_cf, var);
    
    if (neg == pos) cf = neg; //no need to create new node, same tree
    else
    {
        cf = tables.find_in_unique_table(np->var, neg, pos); //check to see if already exists
        if (cf != 0) return cf; //already exists
        else //create node
        {
            cf = tables.create_and_add_to_unique_table(np->var, neg, pos);
            cf->probability = (neg->probability + pos->probability)/2; //prob
        }
    }
    return cf;
}

// boolean_difference takes the BDD pointed to by np,
// and returns the boolean difference with respect to var.
bdd_ptr boolean_difference(bdd_ptr np, char var)
{
    // get reference to tables
    //bdd_tables& tables = bdd_tables::getInstance();
    
    //... your code goes here
    //boolean diff = pos cofactor XOR neg cofactor
    return apply(negative_cofactor(np, var), positive_cofactor(np,var), "xor");
}

void find_all_variables(bdd_ptr np, set<char> & out){ //helper function for sort_by influence
    
    if(np->is_terminal()) return;
    find_all_variables(np->neg_cf, out);
    find_all_variables(np->pos_cf, out);
    out.insert(np->var);
}

bdd_ptr sort_by_influence(bdd_ptr np)
{
    set<char> v;
    vector<char> vars;
    find_all_variables(np, v);
    vector<variable> out;
    vars.assign(v.begin(), v.end());
    out.reserve(20);
    for(int i=0; i< (int)vars.size(); ++i){
       // cout << vars[i] << endl;
       bdd_ptr a = boolean_difference(np, vars[i]);
       out.push_back(variable(a->probability, vars[i]));
    }
    sort(out.begin(), out.end(), influence()); 
    for(int i = 0; i < (int)out.size(); ++i){
	cout << out[i].var << "," << out[i].prob << endl;
    } 
    return np;
}


