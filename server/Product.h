#ifndef PRODUCT_H
#define PRODUCT_H
#include <string>


class Product
{
    public:
        Product( int, std::string, int);
        virtual ~Product();
        int Getid() { return id; }
        void Setid(int val) { id = val; }
        int GetcurrBid() { return currBid; }
        void SetcurrBid(int val) { currBid = val; }
        int Getquantity() { return quantity; }
        void Setquantity(int val) { quantity = val; }
        int GetlastbidID() { return lastbidID;}
        void SetlastbidID( int val) { lastbidID = val;}
        int GetmaxBid(){ return maxBid;}
        void SetmaxBid( int val) { maxBid = val;}
        std::string GetprodID() { return prodid;}
        void SetprodID( std::string val) { prodid = val;}
    protected:
    private:
        int id;
        int currBid;
        int quantity;
        int lastbidID;
        int maxBid;
        std::string prodid;
};

#endif // PRODUCT_H

