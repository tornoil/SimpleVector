#pragma once

#include <cassert>
#include <initializer_list>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include "array_ptr.h"

#include <iostream>

struct ReserveProxyObj
{
    size_t capacity_to_reserve_;
    ReserveProxyObj(size_t capacity_to_reserve)
        : capacity_to_reserve_(capacity_to_reserve) {}
};

ReserveProxyObj Reserve(size_t capacity_to_reserve)
{
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector
{
public:
    using Iterator = Type *;
    using ConstIterator = const Type *;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) : SimpleVector(size, 0){
    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type &value){
        
        size_ = size;
        capacity_ = size;
        ArrayPtr<Type> tmp(size);
        items_.swap(tmp);
        std::fill(&items_[0], &items_[size_], value);
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init){
        
        size_ = init.size();
        capacity_ = size_;
        ArrayPtr<Type> tmp(size_);
        std::copy(init.begin(), init.end(), &tmp[0]);
        items_.swap(tmp);
    }
    void Reserve(size_t new_capacity){
        if (new_capacity > capacity_)
        {
            ArrayPtr<Type> tmp(new_capacity);
            std::copy(std::make_move_iterator(&items_[0]), std::make_move_iterator(&items_[size_]), &tmp[0]);
            items_.swap(tmp);
            capacity_ = new_capacity;
        }
    }
    SimpleVector &operator=(const SimpleVector &rhs){
        
        if (this == &rhs)
            return *this;
        SimpleVector<Type> tmp(rhs);
        swap(tmp);
        return *this;
    }
    SimpleVector(const SimpleVector &other){
        
        SimpleVector<Type> tmp(other.GetSize());
        std::copy(other.begin(), other.end(), tmp.begin());
        swap(tmp);
    }

    SimpleVector(SimpleVector &&other){
        
        items_.swap(other.items_);
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.size_ = 0;
        other.capacity_ = 0;
    }
    SimpleVector(ReserveProxyObj cap){
        Reserve(cap.capacity_to_reserve_);
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(Type item){
        if(size_ < capacity_){
            items_[size_] = std::move(item);
            ++size_;
        } else {
            Insert(cend(), std::move(item));
        }
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, Type value)
    {
        size_t dist_to_pos = 0;
        if (size_ < capacity_)
        {
            dist_to_pos = std::distance(cbegin(), pos);
            size_t dist_to_end = std::distance(this->begin(), this->end());
            std::copy_backward(std::make_move_iterator(&items_[dist_to_pos]), std::make_move_iterator(&items_[dist_to_end]), &items_[capacity_]);
            items_[dist_to_pos] = std::move(value);
            ++size_;
        } else {
            int new_size = 0;
            if (size_ == 0){
                new_size = 2;
            } else {
                new_size = size_ * 2;
            }
            ArrayPtr<Type> tmp_array(new_size);
            dist_to_pos = std::distance(this->cbegin(), pos);
            std::copy(std::make_move_iterator(&items_[0]), std::make_move_iterator(&items_[dist_to_pos]), &tmp_array[0]);

            tmp_array[dist_to_pos] = std::move(value);
            size_t dist_to_end = std::distance(this->begin(), this->end());
            std::copy_backward(std::make_move_iterator(&items_[dist_to_pos]), std::make_move_iterator(&items_[dist_to_end]), &tmp_array[dist_to_end + 1]);
            
            items_.swap(tmp_array);
            ++size_;
            ++capacity_;
        }
        return &items_[dist_to_pos];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept
    {
        if (size_ > 0)
        {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos)
    {
        auto pos_non_const = const_cast<Iterator>(pos);
        std::copy(std::make_move_iterator(pos_non_const + 1), std::make_move_iterator(end()), pos_non_const);
        -- size_;
        return pos_non_const;
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector &other) noexcept
    {
        
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
        items_.swap(other.items_);
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept
    {
        
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept
    {
        
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept
    {
        if (size_ == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    // Возвращает ссылку на элемент с индексом index
    Type &operator[](size_t index) noexcept
    {
        
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type &operator[](size_t index) const noexcept
    {
        
        return items_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type &At(size_t index)
    {
        
        if (index < size_)
        {
            return items_[index];
        }
        else
        {
            throw std::out_of_range("out of range");
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type &At(size_t index) const
    {
        if (index < size_)
        {
            return items_[index];
        }
        else
        {
            throw std::out_of_range("out of range");
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept
    {
        
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        
        if (new_size <= size_)
        {
            size_ = new_size;
        }
        else
        {
            ArrayPtr<Type> tmp(new_size);
            //copy
            std::copy(std::make_move_iterator(&items_[0]), std::make_move_iterator(&items_[size_]), &tmp[0]);
            // for(size_t i = 0 ; i < size_; ++i){
            //     tmp[i] = items_[i];
            // }
            std::fill(&tmp[size_], &tmp[new_size], 0);
            items_.swap(tmp);
            size_ = new_size;
            capacity_ = new_size;
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept
    {
        
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept
    {
        
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept
    {
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept
    {
        
        return &items_[size_];
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept
    {
        
        return &items_[0];
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept
    {
        
        return &items_[size_];
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
    void CopyToRight(Iterator dest){

    }
};

template <typename Type>
inline bool operator==(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return std::lexicographical_compare(rhs.begin(), rhs.end(), lhs.begin(), lhs.end());
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type> &lhs, const SimpleVector<Type> &rhs)
{
    // Заглушка. Напишите тело самостоятельно
    return !(lhs < rhs);
}
