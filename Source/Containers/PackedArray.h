#pragma once

#include <vector>

namespace JoeEngine {
    template <typename T>
    class JEPackedArray {
    private:

        // Essentially an integer, but default value is -1 rather than zero
        class JEIDInt {
        public:

            // The id value: -1 means invalid, any other number >= 0 is an index
            int m_index;

            // Used for free-list: contains index of next free JEIDInt in the indirection map
            int m_next;

            JEIDInt() : m_index(-1), m_next(-1) {}
            JEIDInt(int v, int n) : m_index(v), m_next(n) {}
            ~JEIDInt() {}
        };

        std::vector<T> m_data;
        std::vector<JEIDInt> m_indirectionMap;
        uint32_t m_numElements;
        int m_freeListHead;

    public:
        JEPackedArray() : m_numElements(0), m_freeListHead(-1) {}
        ~JEPackedArray() {}

        template <typename T>
        void AddElement(uint32_t index, T element) {
            if (index + 1 > m_indirectionMap.size()) {
                m_indirectionMap.resize(index + 1);
            }

            if (m_freeListHead == -1) {
                m_data.emplace_back(element);
                m_indirectionMap[index] = JEIDInt(m_numElements, -1);
                ++m_numElements;
            } else {
                int newFreeListHead = m_indirectionMap[m_freeListHead].m_next;
                m_indirectionMap[m_freeListHead] = JEIDInt(m_numElements, -1);
                ++m_numElements;
                m_freeListHead = newFreeListHead;
            }
        }

        void RemoveElement(uint32_t index) {
            if (index + 1 > m_indirectionMap.size()) {
                // TODO: throw?
                return;
            }

            if (index + 1 == m_numElements) {
                // Invariant: this must be the first element to be removed
                m_indirectionMap[index].m_index = -1;
                m_indirectionMap[index].m_next = -1;
                m_freeListHead = index;
                --m_numElements;
            } else {
                std::swap(m_data[index], m_data[m_numElements - 1]);
                m_indirectionMap[index].m_index = -1;
                m_indirectionMap[index].m_next = m_freeListHead;
                m_freeListHead = index;
                --m_numElements;
            }
        }
    };
}
