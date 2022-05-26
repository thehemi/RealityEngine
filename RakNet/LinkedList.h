/*	
(Circular) Linked List ADT (Doubly Linked Pointer to Node Style) - By Kevin Jenkins (http://www.rakkar.org)
Initilize with the following command
LinkedList<TYPE>
OR
CircularLinkedList<Type>

Has the following member functions
size - returns number of elements in the linked list
insert(item) - inserts <item> at the current position in the LinkedList.
add(item) - inserts <item> after the current position in the LinkedList.  Does not increment the position
replace(item) - replaces the element at the current position <item>.
peek - returns the element at the current position
pop - returns the element at the current position and deletes it
del - deletes the current element.  Does nothing for an empty list.
clear - empties the LinkedList and returns storage
bool is_in(item) - Does a linear search for <item>.  Does not set the position to it, only returns true on item found, false otherwise
bool find(item) - Does a linear search for <item> and sets the current position to point to it if and only if the item is found.  Returns true on item found, false otherwise
sort - Sorts the elements of the list with a mergesort and sets the current pointer to the first element
concatenate (<list> L) // This appends L to the current list
++ (prefix) - moves the pointer one element up in the list and returns the appropriate copy of the element in the list
-- (prefix) - moves the pointer one element back in the list and returns the appropriate copy of the element in the list
beginning - moves the pointer to the start of the list.  For circular linked lists this is first 'position' created.  You should call this after the sort function to read the first value.
end - moves the pointer to the end of the list.  For circular linked lists this is one less than the first 'position' created

The assignment and copy constructor operators are defined

Notes:
1. LinkedList and CircularLinkedList are exactly the same except LinkedList won't let you wrap around the root and lets you jump to two positions relative to the root/
2. Postfix ++ and -- can be used but simply call the prefix versions.


EXAMPLE:

	LinkedList<int> A;  // Creates a Linked List of integers called A
	CircularLinkedList<int> B;  // Creates a Circular Linked List of integers called B

	A.insert(20);  // Adds 20 to A.  A: 20 - current is 20
	A.insert(5);  // Adds 5 to A.  A: 5 20 - current is 5
	A.insert(1);  // Adds 1 to A.  A: 1 5 20 - current is 1

	A.is_in(1); // returns true
	A.is_in(200); // returns false
	A.find(5);  // returns true and sets current to 5
	A.peek();  // returns 5
	A.find(1);  // returns true and sets current to 1

	(++A).peek();  // Returns 5
	A.peek(); // Returns 5

	A.replace(10);  // Replaces 5 with 10.
	A.peek();  // Returns 10

	A.beginning();  // Current points to the beginning of the list at 1

	(++A).peek();  // Returns 5
	A.peek();  // Returns 10

	A.del();  // Deletes 10.  Current points to the next element, which is 20
	A.peek();  // Returns 20

	A.beginning();  // Current points to the beginning of the list at 1

	(++A).peek();  // Returns 5
	A.peek();  // Returns 20

	A.clear();  // Deletes all nodes in A
	
	A.insert(5);  // A: 5 - current is 5
	A.insert(6); // A: 6 5 - current is 6
	A.insert(7); // A: 7 6 5 - current is 7

	A.clear();
	B.clear();

	B.add(10);
	B.add(20);
	B.add(30);
	B.add(5);
	B.add(2);
	B.add(25);

	B.sort();  // Sorts the numbers in the list and sets the current pointer to the first element

    // Postfix ++ just calls the prefix version and has no functional difference.
	B.peek();  // Returns 2
	B++;
	B.peek();  // Returns 5
	B++;
	B.peek();  // Returns 10
	B++;
	B.peek();  // Returns 20
	B++;
	B.peek();  // Returns 25
	B++;
	B.peek();  // Returns 30


*/

#ifndef __LINKED_LIST_H
#define __LINKED_LIST_H

namespace BasicDataStructures
{
	
// Prototype to prevent error in CircularLinkedList class when a reference is made to a LinkedList class
template <class LinkedListType>
class LinkedList;

template <class CircularLinkedListType>
class CircularLinkedList
{
	public:
		struct node
		{
			CircularLinkedListType item;

			node* previous;
			node* next;
		};

		CircularLinkedList();
		~CircularLinkedList();
		CircularLinkedList(const CircularLinkedList& original_copy);
		// CircularLinkedList(LinkedList<CircularLinkedListType> original_copy) {CircularLinkedList(original_copy);}  // Converts linked list to circular type
		bool operator= (const CircularLinkedList& original_copy);
		CircularLinkedList& operator++();  // CircularLinkedList A; ++A;
		CircularLinkedList& operator++(int);  // Circular_Linked List A; A++;
		CircularLinkedList& operator--();  // CircularLinkedList A; --A;
		CircularLinkedList& operator--(int);  // Circular_Linked List A; A--;
		bool is_in(const CircularLinkedListType& input);
		bool find(const CircularLinkedListType& input);
		void insert(const CircularLinkedListType& input);
		CircularLinkedListType& add(const CircularLinkedListType& input); // Adds after the current position
		void replace(const CircularLinkedListType& input);
		void del(void);
		unsigned long size(void);
		CircularLinkedListType& peek(void);
		const CircularLinkedListType pop(void);
		void clear(void);
		void sort(void);
		void beginning(void);
		void end(void);
		void concatenate(const CircularLinkedList& L);
								
	protected:
		unsigned long list_size;
		node *root; node *position;
		node* find_pointer(const CircularLinkedListType& input);

	private:
		CircularLinkedList merge(CircularLinkedList L1, CircularLinkedList L2);
		CircularLinkedList mergesort(const CircularLinkedList& L);
};

template <class LinkedListType>
class LinkedList : public CircularLinkedList<LinkedListType>
{
	public:
		LinkedList() {}
		LinkedList(const LinkedList& original_copy);
		~LinkedList();
		bool operator= (const LinkedList<LinkedListType>& original_copy);
		LinkedList& operator++();  // LinkedList A; ++A;
		LinkedList& operator++(int);  // Linked List A; A++;
		LinkedList& operator--();  // LinkedList A; --A;
		LinkedList& operator--(int);  // Linked List A; A--;
	private:
		LinkedList merge(LinkedList L1, LinkedList L2);
		LinkedList mergesort(const LinkedList& L);

};


template <class CircularLinkedListType>
inline void CircularLinkedList<CircularLinkedListType>::beginning(void)
{
	if (root)
		position=root;
}

template <class CircularLinkedListType>
inline void CircularLinkedList<CircularLinkedListType>::end(void)
{
	if (root)
		position=root->previous;
}

template <class LinkedListType>
bool LinkedList<LinkedListType>::operator= (const LinkedList<LinkedListType>& original_copy)
{
	typename LinkedList::node *original_copy_pointer, *last, *save_position;

	if ((&original_copy) != this)
	{
	
	clear();


	if (original_copy.list_size==0L) 
	{
		root=0;
		position=0;
		list_size=0L;
	}
	else if (original_copy.list_size==1L)
	{
		root= new typename LinkedList::node;
		// root->item = new LinkedListType;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
	}
	else
	{
		// Setup the first part of the root node
		original_copy_pointer=original_copy.root;
		root = new typename LinkedList::node;
		// root->item = new LinkedListType;
		position = root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
		if (original_copy_pointer == original_copy.position) save_position=position;

		do
		{
			

			// Save the current element
			last=position;

			// Point to the next node in the source list
			original_copy_pointer=original_copy_pointer->next;

			// Create a new node and point position to it
			position = new typename LinkedList::node;
			// position->item = new LinkedListType;

			// Copy the item to the new node
			// *(position->item)=*(original_copy_pointer->item);
			position->item = original_copy_pointer->item;
			if (original_copy_pointer == original_copy.position) save_position=position;


			// Set the previous pointer for the new node
			(position->previous) = last;

			// Set the next pointer for the old node to the new node
			(last->next) =position;

		} while ((original_copy_pointer->next) != (original_copy.root));

		// Complete the circle.  Set the next pointer of the newest node to the root and the previous pointer of the root to the newest node
		position->next=root;
		root->previous=position;

		list_size=original_copy.list_size;
		position=save_position;
	}
}

return true;
}


template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>::CircularLinkedList()
{
	root=0;
	position=0;
	list_size=0L;
}

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>::~CircularLinkedList()
{
	clear();
}

template <class LinkedListType>
LinkedList<LinkedListType>::~LinkedList()
{
	clear();
}

template <class LinkedListType>
LinkedList<LinkedListType>::LinkedList(const LinkedList& original_copy)
{
	typename LinkedList::node *original_copy_pointer,*last, *save_position;

	if (original_copy.list_size==0L) 
	{
		root=0;
		position=0;
		list_size=0L;
		return;
	}
	else if (original_copy.list_size==1L)
	{
		root= new typename LinkedList::node;
		// root->item = new CircularLinkedListType;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
		// *(root->item) = *((original_copy.root)->item);
		root->item = original_copy.root->item;
	}
	else
	{
		// Setup the first part of the root node
		original_copy_pointer=original_copy.root;
		root = new typename LinkedList::node;
		// root->item = new CircularLinkedListType;
		position = root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
		if (original_copy_pointer == original_copy.position) save_position=position;

		do
		{
			// Save the current element
			last=position;

			// Point to the next node in the source list
			original_copy_pointer=original_copy_pointer->next;

			// Create a new node and point position to it
			position = new typename LinkedList::node;
			// position->item = new CircularLinkedListType;

			// Copy the item to the new node
			// *(position->item)=*(original_copy_pointer->item);
			position->item = original_copy_pointer->item;
			if (original_copy_pointer == original_copy.position) save_position=position;

			// Set the previous pointer for the new node
			(position->previous) = last;

			// Set the next pointer for the old node to the new node
			(last->next) =position;

		} while ((original_copy_pointer->next) != (original_copy.root));

		// Complete the circle.  Set the next pointer of the newest node to the root and the previous pointer of the root to the newest node
		position->next=root;
		root->previous=position;

		list_size=original_copy.list_size;
		position=save_position;
	}
}

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>::CircularLinkedList(const CircularLinkedList& original_copy)
{
	node *original_copy_pointer; node *last; node *save_position;

	if (original_copy.list_size==0L) 
	{
		root=0;
		position=0;
		list_size=0L;
		return;
	}
	else if (original_copy.list_size==1L)
	{
		root= new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
		// *(root->item) = *((original_copy.root)->item);
		root->item = original_copy.root->item;
	}
	else
	{
		// Setup the first part of the root node
		original_copy_pointer=original_copy.root;
		root = new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		position = root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
		if (original_copy_pointer == original_copy.position) save_position=position;

		do
		{
			

			// Save the current element
			last=position;

			// Point to the next node in the source list
			original_copy_pointer=original_copy_pointer->next;

			// Create a new node and point position to it
			position = new typename CircularLinkedList::node;
			// position->item = new CircularLinkedListType;

			// Copy the item to the new node
			// *(position->item)=*(original_copy_pointer->item);
			position->item = original_copy_pointer->item;
			if (original_copy_pointer == original_copy.position) save_position=position;

			// Set the previous pointer for the new node
			(position->previous) = last;

			// Set the next pointer for the old node to the new node
			(last->next) =position;

		} while ((original_copy_pointer->next) != (original_copy.root));

		// Complete the circle.  Set the next pointer of the newest node to the root and the previous pointer of the root to the newest node
		position->next=root;
		root->previous=position;

		list_size=original_copy.list_size;
		position=save_position;
	}
}

template <class CircularLinkedListType>
bool CircularLinkedList<CircularLinkedListType>::operator= (const CircularLinkedList& original_copy)
{
	node *original_copy_pointer; node *last; node *save_position;

	if ((&original_copy) != this)
	{
	
	clear();


	if (original_copy.list_size==0L) 
	{
		root=0;
		position=0;
		list_size=0L;
	}
	else if (original_copy.list_size==1L)
	{
		root= new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
	}
	else
	{
		// Setup the first part of the root node
		original_copy_pointer=original_copy.root;
		root = new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		position = root;
		// *(root->item)=*((original_copy.root)->item);
		root->item = original_copy.root->item;
		if (original_copy_pointer == original_copy.position) save_position=position;

		do
		{
			// Save the current element
			last=position;

			// Point to the next node in the source list
			original_copy_pointer=original_copy_pointer->next;

			// Create a new node and point position to it
			position = new typename CircularLinkedList::node;
			// position->item = new CircularLinkedListType;

			// Copy the item to the new node
			// *(position->item)=*(original_copy_pointer->item);
			position->item = original_copy_pointer->item;
			if (original_copy_pointer == original_copy.position) save_position=position;

			// Set the previous pointer for the new node
			(position->previous) = last;

			// Set the next pointer for the old node to the new node
			(last->next) =position;

		} while ((original_copy_pointer->next) != (original_copy.root));

		// Complete the circle.  Set the next pointer of the newest node to the root and the previous pointer of the root to the newest node
		position->next=root;
		root->previous=position;

		list_size=original_copy.list_size;
		position=save_position;
	}
}

return true;
}

template <class CircularLinkedListType>
void CircularLinkedList<CircularLinkedListType>::insert(const CircularLinkedListType& input)
{
	node *new_node;

	if (list_size==0L)
	{
		root= new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		//*(root->item)=input;
		root->item = input;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
	}

	else if (list_size==1L)
	{
		position= new typename CircularLinkedList::node;
		// position->item = new CircularLinkedListType;
		root->next=position;
		root->previous=position;
		position->previous=root;
		position->next=root;
		// *(position->item)=input;
		position->item = input;
		root=position;  // Since we're inserting into a 1 element list the old root is now the second item
		list_size=2L;
	}

	else
	{
		/*

		    B
		    |
		 A --- C
		 		 
		 position->previous=A
		 new_node=B
		 position=C

		Note that the order of the following statements is important  */
		
		new_node = new typename CircularLinkedList::node;
		// new_node->item = new CircularLinkedListType;

		// *(new_node->item)=input;
		new_node->item = input;

		// Point next of A to B
		(position->previous)->next=new_node;

		// Point last of B to A
		new_node->previous=position->previous;

		// Point last of C to B
		position->previous=new_node;

		// Point next of B to C
		new_node->next=position;

		 // Since the root pointer is bound to a node rather than an index this moves it back if you insert an element at the root
		if (position==root) {root=new_node; position=root;}

		// Increase the recorded size of the list by one
		list_size++;
	}
}

template <class CircularLinkedListType>
CircularLinkedListType& CircularLinkedList<CircularLinkedListType>::add(const CircularLinkedListType& input)
{
	node *new_node;

	if (list_size==0L)
	{
		root= new typename CircularLinkedList::node;
		// root->item = new CircularLinkedListType;
		// *(root->item)=input;
		root->item=input;
		root->next=root;
		root->previous=root;
		list_size=1L;
		position=root;
		// return *(position->item);
		return position->item;
	}

	else if (list_size==1L)
	{
		position= new typename CircularLinkedList::node;
		// position->item = new CircularLinkedListType;
		root->next=position;
		root->previous=position;
		position->previous=root;
		position->next=root;
		// *(position->item)=input;
		position->item = input;
		list_size=2L;
		position=root;  // Don't move the position from the root
		// return *(position->item);
		return position->item;
	}

	else
	{
		/*

		    B
		    |
		 A --- C
		 		 
		 new_node=B
		 position=A
		 position->next=C

		Note that the order of the following statements is important  */
		
		new_node = new typename CircularLinkedList::node;
		// new_node->item = new CircularLinkedListType;

		// *(new_node->item)=input;
		new_node->item=input;

		// Point last of B to A
		new_node->previous=position;

		// Point next of B to C
		new_node->next=(position->next);

		// Point last of C to B
		(position->next)->previous=new_node;

		// Point next of A to B
		(position->next)=new_node;

		// Increase the recorded size of the list by one
		list_size++;
		
		// return *(new_node->item);
		return new_node->item;
	}
}

template <class CircularLinkedListType>
inline void CircularLinkedList<CircularLinkedListType>::replace(const CircularLinkedListType& input)
{
	if (list_size>0L)
		// *(position->item)=input;
		position->item=input;
}

template <class CircularLinkedListType>
void CircularLinkedList<CircularLinkedListType>::del()
{
	node* new_position;

	if (list_size==0L) return;

	else if (list_size==1L)
	{
		// delete root->item;
		delete root;
		root=position=0;
		list_size=0L;
	}

	else
	{
		(position->previous)->next = position->next;
		(position->next)->previous = position->previous;
		new_position=position->next;
		if (position==root) root=new_position;
		// delete position->item;
		delete position;
		position=new_position;
		list_size--;
	}
}

template <class CircularLinkedListType>
bool CircularLinkedList<CircularLinkedListType>::is_in(const CircularLinkedListType& input)
{
	node *return_value,*old_position;
	
	old_position=position;
 
	return_value=find_pointer(input);
	position=old_position;

	if (return_value!=0) return true;
	else return false;	// Can't find the item don't do anything
}

template <class CircularLinkedListType>
bool CircularLinkedList<CircularLinkedListType>::find(const CircularLinkedListType& input)
{
	node* return_value;
 
	return_value=find_pointer(input);

	if (return_value!=0) {position=return_value; return true;}
	else return false;	// Can't find the item don't do anything
}

template <class CircularLinkedListType>
typename CircularLinkedList<CircularLinkedListType>::node* CircularLinkedList<CircularLinkedListType>::find_pointer(const CircularLinkedListType& input)
{
	node* current;

	if (list_size==0L) return 0;
	current=root;

	// Search for the item starting from the root node and incrementing the pointer after every check
	// If you wind up pointing at the root again you looped around the list so didn't find the item, in which case return 0
	do
	{
		// if (*(current->item) == input) return current;
		if (current->item == input) return current;
		current=current->next;
	} while (current != root);

	return 0;
	
}

template <class CircularLinkedListType>
inline unsigned long CircularLinkedList<CircularLinkedListType>::size(void)
{
	return list_size;
}

template <class CircularLinkedListType>
inline CircularLinkedListType& CircularLinkedList<CircularLinkedListType>::peek(void)
{
	// return *(position->item);
	return position->item;
}

template <class CircularLinkedListType>
const CircularLinkedListType CircularLinkedList<CircularLinkedListType>::pop(void)
{
	CircularLinkedListType element;
	element = peek();
 	del();
	return CircularLinkedListType(element); // return temporary
}

// Prefix
template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator++()
{
	if (list_size!=0L) position=position->next;
	return *this;
}

/*
// Postfix
template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator++(int)
{
	CircularLinkedList<CircularLinkedListType> before;
	before=*this;
	operator++();
	return before;
}
*/

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator++(int)
{
	return operator++();
}

// Prefix
template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator--()
{
	if (list_size!=0L) position=position->previous;
	return *this;
}

/*
// Postfix
template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator--(int)
{
	CircularLinkedList<CircularLinkedListType> before;
	before=*this;
	operator--();
	return before;
}
*/

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType>& CircularLinkedList<CircularLinkedListType>::operator--(int)
{
	return operator--();
}

template <class CircularLinkedListType>
void CircularLinkedList<CircularLinkedListType>::clear(void)
{
	if (list_size==0L) return;
	else if (list_size==1L)// {delete root->item; delete root;}
		{delete root;}
	else
	{
		node* current;
		node* temp;

		current=root;

		do
		{
			temp=current;
			current=current->next;
		//	delete temp->item;
			delete temp;
		} while (current!=root);
	}

	list_size=0L;
	root=0;
	position=0;
}

template <class CircularLinkedListType>
inline void CircularLinkedList<CircularLinkedListType>::concatenate(const CircularLinkedList<CircularLinkedListType>& L)
{
	unsigned long counter;
	node* ptr;

	if (L.list_size == 0L) return;
	if (list_size == 0L) *this = L;

	ptr = L.root;
	position=root->previous;
	
	// Cycle through each element in L and add it to the current list
	for (counter = 0; counter < L.list_size; counter++)
	{
		// Add item after the current item pointed to
		// add(*(ptr->item));
		add (ptr->item);

		// Update pointers.  Moving ptr keeps the current pointer at the end of the list since the add function does not move the pointer
		ptr=ptr->next;
		position=position->next;
	}
}

template <class CircularLinkedListType>
inline void CircularLinkedList<CircularLinkedListType>::sort(void)
{
	if (list_size<=1L) return;

	// Call equal operator to assign result of mergesort to current object
	*this = mergesort(*this);
	position=root;
}

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType> CircularLinkedList<CircularLinkedListType>::mergesort(const CircularLinkedList& L)
{
	unsigned long counter;
	node* location;
	CircularLinkedList<CircularLinkedListType> L1;
	CircularLinkedList<CircularLinkedListType> L2;

	location=L.root;

	// Split the list into two equal size sublists, L1 and L2

	for (counter=0L; counter < L.list_size / 2L; counter++)
	{
		// L1.add (*(location->item));
		L1.add (location->item);
		location = location->next;
	}

	for (;counter < L.list_size; counter++)
	{
		// L2.add(*(location->item));
		L2.add (location->item);
		location=location->next;
	}	
	
	// Recursively sort the sublists
	if (L1.list_size > 1) L1=mergesort(L1);
	if (L2.list_size > 1) L2=mergesort(L2);

	// Merge the two sublists
	return merge(L1, L2);
}

template <class CircularLinkedListType>
CircularLinkedList<CircularLinkedListType> CircularLinkedList<CircularLinkedListType>::merge(CircularLinkedList L1, CircularLinkedList L2)
{
	CircularLinkedList<CircularLinkedListType> X;
	CircularLinkedListType element;
	L1.position = L1.root;  L2.position = L2.root;

	// While neither list is empty
	while ((L1.list_size != 0L) && (L2.list_size != 0L))
	{
		// Compare the first items of L1 and L2
		// Remove the smaller of the two items from the list

		if (((L1.root)->item) < ((L2.root)->item))
		// if ((*((L1.root)->item)) < (*((L2.root)->item)))
		{
			// element = *((L1.root)->item);
			element = (L1.root)->item;
			L1.del();
		}
		else
		{
			// element = *((L2.root)->item);
			element = (L2.root)->item;
			L2.del();
		}

		// Add this item to the end of X
		X.add(element);
		X++;
	}

	// Add the remaining list to X
	if (L1.list_size != 0L) X.concatenate(L1);
	else X.concatenate(L2);

	return X;
}

template <class LinkedListType>
LinkedList<LinkedListType> LinkedList<LinkedListType>::mergesort(const LinkedList& L)
{
	unsigned long counter;
	typename LinkedList::node* location;
	LinkedList<LinkedListType> L1;
	LinkedList<LinkedListType> L2;

	location=L.root;

	// Split the list into two equal size sublists, L1 and L2

	for (counter=0L; counter < L.LinkedList_size / 2L; counter++)
	{
		// L1.add (*(location->item));
		L1.add (location->item);
		location = location->next;
	}

	for (;counter < L.LinkedList_size; counter++)
	{
		// L2.add(*(location->item));
		L2.add (location->item);
		location=location->next;
	}	
	
	// Recursively sort the sublists
	if (L1.list_size > 1) L1=mergesort(L1);
	if (L2.list_size > 1) L2=mergesort(L2);

	// Merge the two sublists
	return merge(L1, L2);
}

template <class LinkedListType>
LinkedList<LinkedListType> LinkedList<LinkedListType>::merge(LinkedList L1, LinkedList L2)
{
	LinkedList<LinkedListType> X;
	LinkedListType element;
	L1.position = L1.root;  L2.position = L2.root;

	// While neither list is empty
	while ((L1.LinkedList_size != 0L) && (L2.LinkedList_size != 0L))
	{
		// Compare the first items of L1 and L2
		// Remove the smaller of the two items from the list

		if (((L1.root)->item) < ((L2.root)->item))
		// if ((*((L1.root)->item)) < (*((L2.root)->item)))
		{
			element = (L1.root)->item;
			// element = *((L1.root)->item);
			L1.del();
		}
		else
		{
			element = (L2.root)->item;
			// element = *((L2.root)->item);
			L2.del();
		}

		// Add this item to the end of X
		X.add(element);
	}

	// Add the remaining list to X
	if (L1.LinkedList_size != 0L) X.concatenate(L1);
	else X.concatenate(L2);

	return X;
}


// Prefix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator++()
{
	if ((list_size!=0L) && (position->next!=root)) position=position->next;
	return *this;
}
/*
// Postfix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator++(int)
{
	LinkedList<LinkedListType> before;
	before=*this;
	operator++();
	return before;
}
*/
// Postfix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator++(int)
{
	return operator++();
}

// Prefix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator--()
{
	if ((list_size!=0L) && (position!=root)) position=position->previous;
	return *this;
}
/*
// Postfix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator--(int)
{
	LinkedList<LinkedListType> before;
	before=*this;
	operator--();
	return before;
}
*/

// Postfix
template <class LinkedListType>
LinkedList<LinkedListType>& LinkedList<LinkedListType>::operator--(int)
{
	return operator--();
}

} // End namespace

#endif
