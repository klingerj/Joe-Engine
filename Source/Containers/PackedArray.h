#pragma once

#include <vector>
#include <iterator>

/* For reference:
http://bitsquid.blogspot.com/2011/09/managing-decoupling-part-4-id-lookup.html
*/

namespace JoeEngine {
    template <typename T>
    class PackedArray {
    private:

        class JEIDInt {
        public:

            // The id value: -1 means invalid, any other number >= 0 is an index
            int m_index;

            JEIDInt() : m_index(-1) {}
            JEIDInt(int v) : m_index(v) {}
            ~JEIDInt() {}
        };

        std::vector<T> m_data; // Packed data
        std::vector<JEIDInt> m_indirectionMap; // Indirection layer between global indices and the data
        std::vector<int> m_dataIndices;
        uint32_t m_numElements;

    public:
        PackedArray() : m_numElements(0) {}
        ~PackedArray() {}

        typename std::vector<T>::iterator begin() noexcept {
            return m_data.begin();
        }

        typename std::vector<T>::iterator end() noexcept {
            return m_data.begin() + m_numElements;
        }

        uint32_t Size() const {
            return m_numElements;
        }

        const std::vector<T>& GetData() const {
            return m_data;
        }

        void AddElement(uint32_t index, T element) {
            if (index + 1 > m_indirectionMap.size()) {
                m_indirectionMap.resize(index + 1);
            }

            m_indirectionMap[index] = JEIDInt(m_numElements);
            if (m_numElements == m_data.size()) {
                m_data.emplace_back(element);
                m_dataIndices.emplace_back(index);
            } else {
                m_data[m_numElements] = element;
                m_dataIndices[m_numElements] = index;
            }
            ++m_numElements;
        }

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

        T& operator[](int i) {
            if (i < 0 || i >= m_indirectionMap.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            if (m_indirectionMap[i].m_index < 0 || m_indirectionMap[i].m_index >= m_data.size()) {
                throw std::runtime_error("Index out of bounds");
            }

            return m_data[m_indirectionMap[i].m_index];
        }

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
