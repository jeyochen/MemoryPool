    使用queue、map实现了一个固定内存块大小的内存池，程序启动时创建指定大小和指定数量的内存块后，即可循环使用这些内存。
queue里存放了每块内存的索引，map里存放了内存索引和内存块的首地址，之所以采用map的方式就是避免使用者重复free同一个
内存块可能会引起的同一个内存块的重复分配。
    vector里存放了内存块的智能指针，主要用途为让系统自动清理内存。