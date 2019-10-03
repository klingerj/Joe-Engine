#include "PackedArray.h"

namespace JoeEngine {

    /*template <typename T>
    void JEPackedArray<T>::AddElement(uint32_t index, T element) {
        if (index + 1 > m_indirectionMap.size()) {
            m_indirectionMap.resize(index + 1);
        }

        if (m_freeListHead == -1) {
            m_data.emplace_back(element);
            ++m_numElements;
            m_indirectionMap[index] = JEIDInt(m_numElements - 1, -1);
        } else {
            int newFreeListHead = m_indirectionMap[m_freeListHead].m_next;
            m_indirectionMap[m_freeListHead] = JEIDInt(m_numElements, -1);
            ++m_numElements;
            m_freeListHead = newFreeListHead;
        }
    }

    template <typename T>
    void JEPackedArray<T>::RemoveElement(uint32_t index) {
        if (index + 1 > m_indirectionMap.size()) {
            // TODO: throw?
            return;
        }
        
        if (index + 1 == m_numElements) {
            // Invariant: this must be the first element to be removed
            m_indirectionMap[index].m_value = -1;
            m_indirectionMap[index].m_next = -1;
            m_freeListHead = index;
            --m_numElements;
        } else {
            std::swap(m_data[index], m_data[m_numElements - 1]);
            m_indirectionMap[index].m_value = -1;
            m_indirectionMap[index].m_next = m_freeListHead;
            m_freeListHead = index;
            --m_numElements;
        }
    }*/
}
