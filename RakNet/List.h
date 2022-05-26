/*	
List ADT (Array Style) - By Kevin Jenkins
Initilize with the following structure
List<TYPE, OPTIONAL INITIAL ALLOCATION SIZE>

Has the following member functions
[x] - Overloaded: Returns element x (starting at 0) in the list between the brackets.  If no x argument is specified it returns the last element on the list. If x is invalid it returns 0
size - returns number of elements in the list
insert(item, x) - inserts <item> at position x in the list.  If no x argument is specified it adds item to the end of the list
replace(item, filler, x) - replaces the element at position x with <item>.  If x is greater than the current list size, the list is increased and the other values are assigned filler
del(OPTIONAL x) - deletes the element at position x.  If no x argument is specified it deletes the last element on the list
compress - reallocates memory to fit the number of elements.  Best used when the number of elements decreases
clear - empties the list and returns storage
The assignment and copy constructor operators are defined

EXAMPLE
List<int, 20> A;
A.size;  // Returns 0
A.insert(10);  // Adds 10 to the end of the list
A[0];  // Returns 10
A.insert(1,0);  // Adds 1 to front of list so list reads 1,10
A.replace(5,0, 1); // Replaces element 1 with a 5.  List reads 1,5
A.del();  // Deletes the last item on the list.  List reads 1
A.size;  // Returns 1

NOTES
The default initial allocation size is 1
This function doubles the amount of memory allocated when the list is filled
This list is held in an array and is best used for random element selection

*/

#include <assert.h>

////#include "MemoryManager.h"

#ifndef __LIST_H
#define __LIST_H

static const unsigned long MAX_UNSIGNED_LONG=4294967295U;
namespace BasicDataStructures
{
template <class list_type>
class List
{
	private:
		list_type* array;
		unsigned long list_size;
		unsigned long allocation_size;
		
	public:
		List();
		~List();
		List(const List& original_copy);
		List& operator= (const List& original_copy);
		list_type& operator[] (unsigned long position);
		void insert(list_type input, unsigned long position);
		void insert(list_type input); // inserts at end
		void replace(list_type input, list_type filler, unsigned long position);
		void replace(list_type input);
		void del(unsigned long position);
		void del();
		// Returns the index of the specified item or MAX_UNSIGNED_LONG if not found
		unsigned long getIndexOf(list_type input);
		const unsigned long size(void);
		void clear(void);
		void compress(void);
};

template <class list_type>
List<list_type>::List()
{
	allocation_size=1;
	array = new list_type[allocation_size];
	list_size=0L;	
}

template <class list_type>
List<list_type>::~List()
{
	clear();
}


template <class list_type>
List<list_type>::List(const List& original_copy)
{
	// Allocate memory for copy
	if (original_copy.list_size==0) {list_size=0L; allocation_size=0L;}
	else
	{
		array = new list_type [original_copy.list_size];

		for (unsigned long counter=0L; counter < original_copy.list_size; ++counter)
			array[counter] = original_copy.array[counter];

		list_size=allocation_size=original_copy.list_size;
	}
}

template <class list_type>
List<list_type>& List<list_type>::operator= (const List& original_copy)
{
	if ((&original_copy) != this)
	{
		clear();
		
		// Allocate memory for copy
		if (original_copy.list_size==0) {list_size=0L; allocation_size=0L;}
		else
		{
			array = new list_type [original_copy.list_size];

			for (unsigned long counter=0L; counter < original_copy.list_size; ++counter)
				array[counter] = original_copy.array[counter];
		
			list_size=allocation_size=original_copy.list_size;
		}
	}
	
	return *this;
}


template <class list_type>
inline list_type& List<list_type>::operator[] (unsigned long position)
{
	if (position >= list_size)
	{
		assert(0); // Reading past the end of the list
		return array[0];
	}
	return array[position];
}

template <class list_type>
void List<list_type>::insert(list_type input, unsigned long position)
{
	#ifdef _DEBUG
	assert(position <= list_size);
	#endif

	// Reallocate list if necessary
	if (list_size==allocation_size)
	{
		// allocate twice the currently allocated memory
		list_type* new_array;

		if (allocation_size==0L) allocation_size=1L;
		else allocation_size*=2L;
	
		new_array = new list_type [allocation_size];

		// copy old array over
		for (unsigned long counter=0L; counter<list_size; ++counter)
			new_array[counter] = array[counter];

		// set old array to point to the newly allocated and twice as large array
		delete[] array;
		array=new_array;
	}

	// Move the elements in the list to make room
	for (unsigned long counter=list_size; counter!=position; counter--)
		array[counter]=array[counter-1];

	// Insert the new item at the correct spot
	array[position]=input;
	++list_size;

}


template <class list_type>
void List<list_type>::insert(list_type input)
{
	// Reallocate list if necessary
	if (list_size==allocation_size)
	{
		// allocate twice the currently allocated memory
		list_type* new_array;

		if (allocation_size==0L) allocation_size=1L;
		else allocation_size*=2L;

		new_array = new list_type [allocation_size];

		// copy old array over
		for (unsigned long counter=0L; counter<list_size; ++counter)
			new_array[counter] = array[counter];

		// set old array to point to the newly allocated and twice as large array
		delete[] array;
		array=new_array;
	}

	// Insert the new item at the correct spot
	array[list_size]=input;
	++list_size;
}

template <class list_type>
inline void List<list_type>::replace(list_type input, list_type filler, unsigned long position)
{
	if ((list_size>0L) && (position<list_size))
	{
		// Direct replacement
		array[position]=input;
	}
	else
	{
		if (position >= allocation_size)
		{
			// Reallocate the list to size position and fill in blanks with filler
			list_type* new_array;
			allocation_size=position+1;
			
			new_array = new list_type [allocation_size];
			
			// copy old array over
			for (unsigned long counter=0L; counter<list_size; ++counter)
				new_array[counter] = array[counter];

			// set old array to point to the newly allocated array
			delete[] array;
			array=new_array;
		}

		// Fill in holes with filler
		while (list_size < position)
			array[list_size++]=filler;

		// Fill in the last element with the new item
		array[list_size++]=input;

		#ifdef _DEBUG
		assert(list_size==position+1);
		#endif
	}
}

template <class list_type>
inline void List<list_type>::replace(list_type input)
{
	if (list_size>0L) array[list_size-1]=input;
}

template <class list_type>
void List<list_type>::del(unsigned long position)
{
	#ifdef _DEBUG
	assert(position < list_size);
	#endif
	if (position < list_size)
	{
		// Compress the array
		for (unsigned long counter=position; counter < list_size-1 ; ++counter)
			array[counter]=array[counter+1L];

		--list_size;
	}
}

template <class list_type>
inline void List<list_type>::del()
{
	// Delete the last element on the list.  No compression needed
	list_size--;
}

template <class list_type>
unsigned long List<list_type>::getIndexOf(list_type input)
{
	for (unsigned long i=0; i < list_size; ++i)
		if (array[i]==input)
			return i;

	return MAX_UNSIGNED_LONG;		
}

template <class list_type>
inline const unsigned long List<list_type>::size(void)
{
	return list_size;
}

template <class list_type>
void List<list_type>::clear(void)
{
	if (allocation_size==0L) return;
	delete [] array;
	list_size=0L;
	allocation_size=0L;
	array=0;
}

template <class list_type>
void List<list_type>::compress(void)
{
	list_type* new_array;

	if (allocation_size==0) return;

	new_array = new list_type [allocation_size];

	// copy old array over
	for (unsigned long counter=0L; counter<list_size; ++counter)
		new_array[counter] = array[counter];

	// set old array to point to the newly allocated array
	delete[] array;
	array=new_array;
}

} // End namespace

#endif
