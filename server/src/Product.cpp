#include "../include/Product.h"

Product::Product( int max, std::string name, int myquantity)
{
    //ctor
    quantity = myquantity;
    currBid = 0;
    maxBid = max;
    prodid = name;
}


Product::~Product()
{
    //dtor
}
