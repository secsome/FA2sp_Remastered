#pragma once

class GenericList;
class GenericNode
{
public:
	explicit GenericNode() noexcept : NextNode(0), PrevNode(0) {}
	virtual ~GenericNode() { Unlink(); }
	GenericNode(GenericNode& another) { another.Link(this); }
	GenericNode& operator = (GenericNode& node)
	{
		if (&node != this)
			node.Link(this);

		return *this;
	}

	void Unlink()
	{
		if (Is_Valid())
		{
			PrevNode->NextNode = NextNode;
			NextNode->PrevNode = PrevNode;
			PrevNode = nullptr;
			NextNode = nullptr;
		}
	}

	GenericList* Main_List() const
	{
		GenericNode const* node = this;
		while (node->PrevNode)
			node = PrevNode;

		return (GenericList*)(this);
	}

	void Link(GenericNode* node)
	{
		node->Unlink();
		node->NextNode = NextNode;
		node->PrevNode = this;
		if (NextNode)
			NextNode->PrevNode = node;
		NextNode = node;
	}

	GenericNode* Next() const { return NextNode; }
	GenericNode* Prev() const { return PrevNode; }
	bool Is_Valid() const { return this != nullptr && NextNode != nullptr && PrevNode != nullptr; }

protected:
	GenericNode* NextNode;
	GenericNode* PrevNode;
};

class GenericList
{
public:
	GenericList() { FirstNode.Link(&LastNode); }

	// Not copyable, but movable
	GenericList(const GenericList& another) = delete;
	GenericList& operator= (const GenericList& another) = delete;
	GenericList(GenericList&& another) = default;
	GenericList& operator= (GenericList&& another) = default;

	GenericNode* First() const { return FirstNode.Next(); }
	GenericNode* Last() const { return LastNode.Prev(); }
	bool Is_Empty() const { return !FirstNode.Next()->Is_Valid(); }
	void Add_Head(GenericNode* node) { FirstNode.Link(node); }
	void Add_Tail(GenericNode* node) { LastNode.Prev()->Link(node); }
	void Delete() { while (FirstNode.Next()->Is_Valid()) delete FirstNode.Next(); }

protected:
	GenericNode FirstNode;
	GenericNode LastNode;
};

template<class T> class List;
template<class T>
class Node : public GenericNode
{
public:
	List<T>* Main_List() const { return (List<T>*)GenericNode::Main_List(); }
	T* Next() const { return((T*)GenericNode::Next()); }
	T* Prev() const { return((T*)GenericNode::Prev()); }
	bool Is_Valid() const { return(GenericNode::Is_Valid()); }
};

template<class T>
class List : public GenericList
{
public:
	T* First() const { return((T*)GenericList::First()); }
	T* Last() const { return((T*)GenericList::Last()); }
};