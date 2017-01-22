#pragma once

template <class T>
class CompareList
{
public:
    CompareList(unsigned int size)
    {
    	this->size = size;
        if (size == 0)
        {
            data = NULL;
            return;
        }
        data = new T[ (size-1)*size/2 ];
    }
    ~CompareList()
    {
    	delete [] data;
    	data = NULL;
    }
    T & operator [] (unsigned int i) const
    {
    	return data[i];
    }
    T & Pair(unsigned int a, unsigned int b) const
    {
     	// no test for out of range error
    	if (a < b)
    	{
    	   	return data[(b-1)*b/2 + a];
    	}
    	else
    	{
            return data[(a-1)*a/2 + b];
    	}
    }
private:
    T *data;
    unsigned int size;
};
