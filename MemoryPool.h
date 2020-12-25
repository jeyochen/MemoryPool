#pragma once

#include <iostream>
#include <string>
#include <queue>
#include <map>
#include <vector>
#include <mutex>
#include <memory>

namespace mempool {
	using ULL = unsigned long long;
	using VOIDPTR = void*;
	using SHORTINT = short int;
	static const short int MEMPOOL_OK = 0;
	static const short int MEMPOOL_PARAMETER_ERROR = 1;
	static const short int MEMPOOL_QUEUE_EMPYT = 2;
	static const short int MEMPOOL_NULL_PTR = 3;
	static const short int MEMPOOL_INVALID_PTR = 4;

	static std::map<SHORTINT, std::string> errStrMap = {
		{MEMPOOL_OK, "�ɹ�"},
		{MEMPOOL_PARAMETER_ERROR, "��������"},
	    {MEMPOOL_QUEUE_EMPYT, "����Ϊ�գ����ڴ�ɹ�����"},
	    {MEMPOOL_NULL_PTR,    "ָ��Ϊ��"},
	    {MEMPOOL_INVALID_PTR, "���ǳ������ָ�룬��Ч"}
	};

	class MemoryPool
	{
	public:
		MemoryPool() = default;
		~MemoryPool() = default;

		// ��ʼ���ڴ��
		int initPool(const size_t blocksize, const size_t blocks);
		
		// ���ڴ�������һ���ڴ湩ʹ��
		int malloc_block(void** ptr);

		// �Ƿ�һ����ʹ����ϵ��ڴ�
		int free_block(void* ptr);

		// ���ݴ����뷵�ش�������
		std::string getErrStr(const SHORTINT n);

		// ���ʼ�ڴ��
		int unInitPool();

	private:
		// һ���ж��ٿ�С�ڴ棬ÿ��Ĵ�СΪblock_size;
		size_t  blocks_ = 0;

		// ÿС���ڴ�Ĵ�С
		size_t  block_size_ = 0;

		// �����ڴ���׵�ַ����ֵ
		std::queue<ULL> block_queue_;

		// �洢���õ��ڴ���׵�ַ����ֵ�͵�ַ
		std::map<ULL, VOIDPTR> block_map_;

		// �������л�����
		std::mutex queue_mtx_;

		// ����map�Ļ�����
		std::mutex map_mtx_;

		// ��������
		std::condition_variable cond_;

		// �洢�ڴ��Ĺ���ָ��
		std::vector<std::shared_ptr<void>> ptr_vec_;
	};
}


