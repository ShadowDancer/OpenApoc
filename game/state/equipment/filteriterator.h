#pragma once
#include "library/sp.h"
#include <functional>
#include <memory>

namespace OpenApoc
{

template <typename TArg> using Predicate = std::function<bool(const TArg)>;

template <typename Iterator>
class FilterIterator
    : public std::iterator<std::forward_iterator_tag, typename Iterator::value_type>
{
  private:
	Iterator pos;
	Iterator end_it;
	Predicate<typename Iterator::value_type> predicate;

  public:
	FilterIterator(Iterator pos, Iterator end_it,
	               Predicate<typename Iterator::value_type> predicate)
	    : pos(pos), end_it(end_it), predicate(predicate)
	{
		auto it = pos;
		while (it != end_it && !predicate(*it))
			++it;
		this->pos = it;
	}

	bool operator!=(const FilterIterator<Iterator> &other) { return this->pos != other.pos; }

	FilterIterator<Iterator> operator++()
	{
		auto it = pos;
		++it;
		while (it != end_it && !predicate(*it))
			++it;
		this->pos = it;
		return FilterIterator<Iterator>{it, this->end_it, predicate};
	};

	typename Iterator::value_type &operator*()
	{
		return reinterpret_cast<typename Iterator::value_type &>(*pos);
	}
};

template <typename Iterator, typename Value>
class TransformIterator
    : public std::iterator<std::forward_iterator_tag, typename Iterator::value_type>
{
  private:
	Iterator pos;

  public:
	TransformIterator(Iterator pos) : pos(pos) {}

	bool operator!=(const TransformIterator<Iterator, Value> &other)
	{
		return this->pos != other.pos;
	}

	TransformIterator<Iterator, Value> operator++()
	{
		auto it = pos;
		this->pos = ++pos;
		return TransformIterator<Iterator, Value>{it};
	};

	sp<Value> operator*()
	{
		// probably should move into template specialized for sp<>
		return std::dynamic_pointer_cast<Value>(*pos);
	}
};

template <typename TContainer, typename TValue> class EquipmentIteratorFactory
{
  private:
	TContainer &container;
	Predicate<typename TContainer::iterator::value_type> predicate;

  public:
	EquipmentIteratorFactory(TContainer &container,
	                         Predicate<typename TContainer::iterator::value_type> predicate)
	    : container(container), predicate(predicate)
	{
	}

	TransformIterator<FilterIterator<typename TContainer::iterator>, TValue> begin()
	{
		return TransformIterator<FilterIterator<typename TContainer::iterator>, TValue>(
		    FilterIterator<typename TContainer::iterator>(container.begin(), container.end(),
		                                                  predicate));
	}

	TransformIterator<FilterIterator<typename TContainer::iterator>, TValue> end()
	{
		return TransformIterator<FilterIterator<typename TContainer::iterator>, TValue>(
		    FilterIterator<typename TContainer::iterator>(container.end(), container.end(),
		                                                  predicate));
	}

	TransformIterator<FilterIterator<typename TContainer::const_iterator>, TValue> cbegin() const
	{
		return TransformIterator<FilterIterator<typename TContainer::const_iterator>, TValue>(
		    FilterIterator<typename TContainer::const_iterator>(container.cbegin(),
		                                                        container.cend(), predicate));
	}

	TransformIterator<FilterIterator<typename TContainer::const_iterator>, TValue> cend() const
	{
		return TransformIterator<FilterIterator<typename TContainer::const_iterator>, TValue>(
		    FilterIterator<typename TContainer::const_iterator>(container.cend(), container.cend(),
		                                                        predicate));
	}
};
} // namespace OpenApoc
