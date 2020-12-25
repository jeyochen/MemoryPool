#include "MemoryPool.h"

#include <chrono>

namespace mempool {
	// ��ʼ���ڴ��
	int MemoryPool::initPool(const size_t blocksize, const size_t blocks) {
		if (0 >= blocksize || 0 >= blocks) {
			return MEMPOOL_PARAMETER_ERROR;
		}

		block_size_ = blocksize;
		blocks_ = blocks;

		std::lock_guard<std::mutex> map_lock(map_mtx_);
		std::lock_guard<std::mutex> queue_lock(queue_mtx_);
		size_t cnt = 0;
		while(cnt < blocks) {
			void* ptr = (void *)(new char[blocksize]);
			if (ptr != nullptr) {
				//��ʼ���ڴ�
				memset(ptr, 0, blocksize);

				// �ѵ�ַת��Ϊ����
				ULL memIdx = reinterpret_cast<ULL>(ptr);

				// ������õ��ڴ��׵�ַ
				block_map_[memIdx] = ptr;

				// ��¼���õ��ڴ������
				block_queue_.push(memIdx);

				// ���칲��ָ�룬�Զ��ͷ��ڴ�
				std::shared_ptr<void> sptr(ptr, [](void* p) {
					std::cout << "delete ptr:" << p << std::endl; 
					delete p;
					p = NULL;
				});
				ptr_vec_.push_back(sptr);

				std::cout << "memory index:" << memIdx << ", addr:" << (void *)ptr << std::endl;
			}
			else {
				std::cout << "new blocksize failed." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(1));
			}
			++cnt;
		}

		return MEMPOOL_OK;
	}

	// ���ڴ�������һ���ڴ湩ʹ��
	int MemoryPool::malloc_block(void** ptr) {

		while (!block_queue_.empty()) {
			// ���������Ƿ����ڴ�ɹ�����
			std::unique_lock<std::mutex>  queue_lock(queue_mtx_);
			if (block_queue_.empty()) {
				std::cout << "block queue empty, malloc memory block failed." << std::endl;
				return MEMPOOL_QUEUE_EMPYT;
			}

			ULL idx = block_queue_.front();
			block_queue_.pop();
			queue_lock.unlock();

			// ���map���Ӧ���ڴ��ַ�Ƿ���Ч
			std::unique_lock<std::mutex>  map_lock(map_mtx_);
			if (block_map_.find(idx) != block_map_.end() && block_map_[idx] != NULL) {
				*ptr = block_map_[idx];
				block_map_[idx] = NULL; // ���ָ���Ѿ������ȥ��
				map_lock.unlock();
				break;
			}
		}

		return MEMPOOL_OK;
	}

	// �ͷ�һ����ʹ����ϵ��ڴ�
	int MemoryPool::free_block(void* ptr) {
		if (NULL == ptr) {
			return MEMPOOL_NULL_PTR;
		}

		// �ȼ���Ƿ��Ǵӳ���������ȥ���ڴ�
		ULL idx = reinterpret_cast<ULL>(ptr);
		std::unique_lock<std::mutex>  map_lock(map_mtx_);
		if (block_map_.find(idx) == block_map_.end()) {
			return MEMPOOL_INVALID_PTR;
		}
		block_map_[idx] = ptr;
		memset(ptr, 0, block_size_);

		std::unique_lock<std::mutex>  queue_lock(queue_mtx_);
		block_queue_.push(idx);

		return MEMPOOL_OK;
	}

    // ���ݴ����뷵�ش�������
	std::string MemoryPool::getErrStr(const SHORTINT n) {
		std::string estr{ "δ֪����" };
		std::map<SHORTINT, std::string>::iterator iter = errStrMap.find(n);
		if (iter != errStrMap.end()) {
			estr = iter->second;
		}

		return estr;
	}

	// ���ʼ�ڴ��
	int MemoryPool::unInitPool() {
		std::unique_lock<std::mutex> qlck(queue_mtx_);
		while (!block_queue_.empty()) {
			block_queue_.pop();
		}
		qlck.unlock();

		std::unique_lock<std::mutex> mlck(map_mtx_);
		block_map_.clear();
		mlck.unlock();

		ptr_vec_.clear();

		return MEMPOOL_OK;
	}
}
