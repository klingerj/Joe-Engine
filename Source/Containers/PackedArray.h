#pragma once

#include <vector>
#include <iterator>

/* For reference:
http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html
*/

namespace JoeEngine {
    //! The PackedArray class.
    /*!
      Data structure class that stores a densely-packed list of data with a variety of helper functions for easy insertion and removal.
      \sa JEEngineInstance
    */
    template <typename T>
    class PackedArray {
    private:

        //! The IDInt class.
        /*!
          Private class solely for usage with a PackedArray. This class essentially just wraps an integer, but ensures that its default
          constructed value is -1 rather than 0. This is important for the PackedArray class because it stores indices - 0 is considered
          a valid index while -1 is not. When the PackedArray grows and resizes, the newly default constructed (but not yet in use)
          index values should contain an invalid value, i.e. -1.
          \sa JEEngineInstance
        */
        class IDInt {
        public:
            //! Constructor.
            /*! Initializes the integer value to -1. */
            IDInt() : m_index(-1) {}

            //! Constructor.
            /*! Initializes the integer value to the specified paramter value. */
            IDInt(int v) : m_index(v) {}

            //! Destructor (default).
            ~IDInt() = default;

            //! Index value.
            /*! The actual integer index value. -1 is invalid, any number >= 0 is a valid index. */
            int m_index;
        };

        //! Data list.
        /*! The densely packed list of data. */
        std::vector<T> m_data;

        //! Indirection map.
        /*! Maps entity IDs (treated as indices) to data in the densely packed list. */
        std::vector<IDInt> m_indirectionMap;

        //! Data-to-indirection indices list.
        /*!
          Parallel to the data list. Each element i stores an index value into the indirection map so that each data element Di
          knows which indirection map index points to it. This is needed when swaps occur in the packed array during removal of
          an element, for example.
        */
        std::vector<int> m_dataIndices;

        //! Number of elements.
        /*! Number of valid elements currently stored in the data list. */
        uint32_t m_numElements;

    public:
        //! Constructor.
        /*! No interesting behavior. */
        PackedArray() : m_numElements(0) {}

        //! Destructor (default).
        ~PackedArray() = default;

        //! Get non-const interator to beginning
        /*!
          Returns a non-const iterator to the front of the underlying data list. This is useful, for example, when using
          a for-each loop to, say, update each element of the array. This is common with various Component types.
          \return iterator to the front of the data list.
          \sa ComponentManager, RotatorComponent
        */
        typename std::vector<T>::iterator begin() noexcept {
            return m_data.begin();
        }

        //! Get non-const interator to end
        /*!
          Returns a non-const iterator to the end of the underlying data list. This is useful, for example, when using
          a for-each loop to, say, update each element of the array. This is common with various Component types.
          \return iterator to the end of the data list.
          \sa ComponentManager, RotatorComponent
        */
        typename std::vector<T>::iterator end() noexcept {
            return m_data.begin() + m_numElements;
        }

        //! Get const interator to beginning
        /*!
          Returns a const iterator to the front of the underlying data list. This is useful, for example, when using
          a for-each loop to read from each element of the array.
          \return iterator to the front of the data list.
          \sa ComponentManager, RotatorComponent
        */
        typename std::vector<T>::const_iterator begin() const noexcept {
            return m_data.begin();
        }

        //! Get const interator to end
        /*!
          Returns a const iterator to the ednd of the underlying data list. This is useful, for example, when using
          a for-each loop to read from each element of the array.
          \return iterator to the end of the data list.
          \sa ComponentManager, RotatorComponent
        */
        typename std::vector<T>::const_iterator end() const noexcept {
            return m_data.begin() + m_numElements;
        }

        //! Get size
        /*!
          Returns the number of data elements stored by this class in the densely-packed data list.
          \return the number of elements
        */
        uint32_t Size() const {
            return m_numElements;
        }

        //! Get const reference to data
        /*!
          Returns a const reference to the data list.
          \return const reference to the data list
        */
        const std::vector<T>& GetData() const {
            return m_data;
        }

        //! Get non-const reference to data
        /*!
          Returns a non-const reference to the data list.
          \return non-const reference to the data list
        */
        std::vector<T>& GetData() {
            return m_data;
        }

        //! Add index-element pair
        /*!
          Adds the specified data element to the densely-packed data list. Will re-use a previously invalid entry if one is available
          to avoid unnecessary allocations. The indirection map and data indices lists are updated to reflect this addition. Now, using
          the []-operator with the specified index will return the specified element.
          \param index the index to insert the element at
          \param element the specified data element
        */
        void AddElement(uint32_t index, T element) {
            if (index + 1 > m_indirectionMap.size()) {
                m_indirectionMap.resize(index + 1);
            }

            m_indirectionMap[index] = IDInt(m_numElements);
            if (m_numElements == m_data.size()) {
                m_data.emplace_back(element);
                m_dataIndices.emplace_back(index);
            } else {
                m_data[m_numElements] = element;
                m_dataIndices[m_numElements] = index;
            }
            ++m_numElements;
        }

        //! Remove element at index
        /*!
          Removes the element at the specified index. The indirection map entry for this index will now be marked as invalid, and future
          attempts to read the data from this index via the []-operator will throw an error.
          \param index the index whose element to remove
        */
        void RemoveElement(uint32_t index) {
            if (index + 1 > m_indirectionMap.size()) {
                // TODO: throw?
                return;
            }

            int dataIdx = m_indirectionMap[index].m_index;
            if (dataIdx == -1) {
                // TODO: throw?
                return;
            } else {
                if (dataIdx + 1 == m_numElements) {
                    // Invariant: this must be the last element in the array
                    m_indirectionMap[index].m_index = -1;
                    m_dataIndices[dataIdx] = -1;
                } else {
                    std::swap(m_data[dataIdx], m_data[m_numElements - 1]);
                    m_indirectionMap[index].m_index = -1;
                    m_indirectionMap[m_dataIndices[m_numElements - 1]].m_index = dataIdx;
                    m_dataIndices[dataIdx] = m_dataIndices[m_numElements - 1];
                    m_dataIndices[m_numElements - 1] = -1;
                }
                --m_numElements;
            }
        }

        //! Get non-const element at index
        /*!
          Accesses the element at the specified index (e.g. an entity ID) via the indirection map. Catches invalid indices and throws an
          error in those cases. This []-operator is for non-const accesses, which is most likely writing.
          \param i the specified index,
          \return a non-const reference to the data element.
        */
        T& operator[](int i) {
            if (i < 0 || i >= m_indirectionMap.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            if (m_indirectionMap[i].m_index < 0 || m_indirectionMap[i].m_index >= m_data.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            return m_data[m_indirectionMap[i].m_index];
        }

        //! Get const element at index
        /*!
          Accesses the element at the specified index (e.g. an entity ID) via the indirection map. Catches invalid indices and throws an
          error in those cases. This []-operator is for const accesses, meaning reading.
          \param i the specified index,
          \return a const reference to the data element.
        */
        const T& operator[](int i) const {
            if (i < 0 || i >= m_indirectionMap.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            if (m_indirectionMap[i].m_index < 0 || m_indirectionMap[i].m_index >= m_data.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            return m_data[m_indirectionMap[i].m_index];
        }
    };
}
