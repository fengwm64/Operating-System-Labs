;;;
; Author: 
; Date: 2023-05-09 17:26:57
; LastEditTime: 2023-06-11 16:06:46
; FilePath: /OSLab/boot/loader.S
; Description: 内核加载器 loader-V5.0 加载内核
; Copyright (c) 2023 by GZHU-FWM, All Rights Reserved. 
;;;

%include "boot.inc"

section loader vstart=LOADER_BASE_ADDR  ; loader在内存中的起始地址
LOADER_STACK_TOP equ LOADER_BASE_ADDR   ; loader保护模式堆栈起始地址也设置为loader起始地址

jmp loader_start            

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 构建 gdt 及其内部的描述符
; ---------------------------------------------------------------------------------   
    ; gdt起始地址
    GDT_BASE:           dd 0x00000000       
                        dd 0x00000000
    ; 代码段描述符
    CODE_DESC:          dd 0x0000FFFF               
                        dd DESC_CODE_HIGH4
    ; 数据段 and 栈段描述符                         
    DATA_STACK_DESC:    dd 0x0000FFFF       
                        dd DESC_DATA_HIGH4
    ; 显存段描述符 
    VIDEO_DESC:         dd 0x80000007       ; limit=(0xbffff-0xb8000)/4k=0x7
                        dd DESC_VIDEO_HIGH4 ; 此时 dpl 为 0                                     

    GDT_SIZE equ $ - GDT_BASE       ; 获得 GDT 的大小
    GDT_LIMIT equ GDT_SIZE - 1      ; 获得段界限

    times 58 dq 0                   ; 将来往 GDT 中添加其他描述符，预留58个描述符的空位         
    times 7 db 0                    ; 7字节，用于对齐，使total_mem_bytes地址为0xb00

; 段选择子
; ---------------------------------------------------------------------------------   
    SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0   ; 相当于(CODE_DESC - GDT_BASE)/8 + TI_GDT + RPL0
    SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0   ; 同上
    SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0  ; 同上

; gdt 指针
; ---------------------------------------------------------------------------------   
    gdt_ptr dw GDT_LIMIT    ; 前 2 字节是 gdt 界限                      
            dd GDT_BASE     ; 后 4 字节是 gdt 起始地址

; 获取内存容量相关
; ---------------------------------------------------------------------------------   
    total_mem_bytes dd 0        ; 用于保存内存容量，单位字节，该变量地址0xb00
    ards_buf times 244 db 0
    ards_nr dw 0                ; 用于记录ARDS结构体数量

; 一些提示信息
; ---------------------------------------------------------------------------------   
    loader_msg db 'loader real-fengWeiMing'    ; 定义字符串，显示即将进入保护模式
    l_len equ $-loader_msg                     ; 计算loader_msg的长度，当前内存位置-message起始位置

    protect_msg db 'protect-fengWeiMing'
    p_len equ $-protect_msg
 
    mem_msg db 'MEM_GET_OK!'
    m_len equ $-mem_msg
 
    error_msg db 'ERROR, HLT!'
    e_len equ $-error_msg

    v_msg db 'V-fengWeiMing'
    v_len equ $-v_msg
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
loader_start:
; 初始化
    mov sp, LOADER_BASE_ADDR    ; 腾出一定的栈空间，以便在执行后面的函数调用时保存寄存器的值
; ---------------------------------------------------------------------------------

; 输出提示信息：loader...
    mov bp, loader_msg		; bp 串首地址
    mov ax, 0x1301    ; AH 功能号=0x13  AL 设置写字符方式=01(显示字符串,光标跟随移动)
    mov bx, 0x70       ; BH 显示的页号=0  BL 字符属性=70h(白底黑字)
    mov dx, 0x0100
    mov cx, l_len   ; CX 串长度=5(不包括结束符0)
    int 0x10          ; 执行BIOS 0x10 号中断
; ---------------------------------------------------------------------------------

; 820获取内存容量
; 输入：
    ; EAX       = 子功能号0xE820
    ; EBX       = ARDX 后续值（e820是迭代方式查询）
    ; ES : DI   = ARDS 缓冲区地址
    ; ECX       = ARDS 结构大小（一般为20字节）
    ; EDX       = 固定的签名标记（规定为0x534d4150，是“SMAP”的ASCII码）
; 输出：
    ; CF        = 值为0调用正常 ; 1调用出错
    ; EAX       = 0x534d4150
    ; ES : DI   = 同输入，ARDS 缓冲区地址，但此时地址指向位置已经填入数据
    ; ECX       = BIOS 写入到 ARDS 缓冲区地址的字节数；最小20字节
    ; EBX       = 后续值，下一个 ARDS 的位置，见输入EBX（迭代）
; ----
; 此中断的调用步骤如下。
    ;（1）填写好“调用前输入”中列出的寄存器。
    ;（2）执行中断调用 int 0x15。
    ;（3）在 CF 位为 0 的情况下，“返回后输出”中对应的寄存器便会有对应的结果。
; ----
    xor ebx, ebx            ; 首次调用，ebx=0
    mov edx, 0x534d4150     ; 固定的签名标记
    mov di, ards_buf

    .e820_mem_get_loop:
        mov eax, 0x0000e820
        mov ecx, 20
        int 0x15
        jc .e820_failed_so_try_e801    ; 若CF位为1,出现错误，尝试e801
        add di, cx
        inc word [ards_nr]
        cmp ebx, 0
        jnz .e820_mem_get_loop
        
        mov cx, [ards_nr]
        mov ebx, ards_buf
        xor edx, edx            ; 清0，edx用于储存最大的内存容量

        .find_max_mem_area:
            mov eax, [ebx]      ; base_add_low
            add eax, [ebx+8]    ; length_low
            add ebx, 20         ; 指向缓冲区中下一个 ARDS 结构
            cmp edx, eax        ; 比较两者大小
            jge .next_ards
            mov edx, eax        ; edx 为总内存大小

        .next_ards:
            loop .find_max_mem_area

        ; 输出提示信息
        mov byte [gs:0xef0],'8'
        mov byte [gs:0xef1],0x30
        mov byte [gs:0xef2],'2'
        mov byte [gs:0xef3],0x30
        mov byte [gs:0xef4],'0'
        mov byte [gs:0xef5],0x30

        jmp .mem_get_ok
; ---------------------------------------------------------------------------------

; 801获取内存容量
; 输入：
    ; AX       = 子功能号0xE801
; 输出：
    ; CF        = 值为0调用正常 ; 1调用出错
    ; AX        = 以1kb为单位，只显示15MB以下内存容量，最大0x3c00(0x3c00*1*1024=15MB)
    ; BX        = 以64kb为单位，显示 16MB ~ 4GB 内存容量，最大0x3c00(0x3c00*64*1024=4GB)
    ; CX        = AX
    ; DX        = BX
; ----
; 此中断的调用步骤如下。
    ;（1）将 AX 寄存器写入 0xE801。
    ;（2）执行中断调用 int 0x15。
    ;（3）在 CF 位为 0 的情况下，“返回后输出”中对应的寄存器便会有对应的结果。
; ----
    .e820_failed_so_try_e801:
        mov ax,0xe801
        int 0x15
        jc .e801_failed_so_try88

        ; 1. 先算出低 15MB 的内存
        ; ax 和 cx 中是以 KB 为单位的内存数量，将其转换为以 byte 为单位
        mov cx, 0x400           ;cx 和 ax 值一样，cx 用作乘数
        mul cx
        shl edx,16
        and eax,0x0000FFFF
        or edx,eax
        add edx, 0x100000       ; ax 只是 15MB，故要加 1MB
        mov esi,edx             ; 先把低 15MB 的内存容量存入 esi 寄存器备份

        ; 2. 再将 16MB 以上的内存转换为 byte 为单位
        ; 寄存器 bx 和 dx 中是以 64KB 为单位的内存数量
        xor eax, eax            ; 清空eax
        mov ax, bx              
        mov ecx, 0x10000        ; 0x10000 十进制为 64KB
        mul ecx                 ; 32 位乘法，默认的被乘数是 eax，积为 64 位
                                ; 高 32 位存入 edx，低 32 位存入 eax
        add esi, eax
        
        ; 由于此方法只能测出 4GB 以内的内存，故 32 位 eax 足够了
        ; edx 肯定为 0，只加 esi 便可
        mov edx, esi            ;edx 为总内存大小

        ; 输出提示信息
        mov byte [gs:0xef0],'8'
        mov byte [gs:0xef1],0x30
        mov byte [gs:0xef2],'0'
        mov byte [gs:0xef3],0x30
        mov byte [gs:0xef4],'1'
        mov byte [gs:0xef5],0x30

        jmp .mem_get_ok
; ---------------------------------------------------------------------------------

; 88获取内存容量
    .e801_failed_so_try88:
        ;int 15 后，ax 存入的是以 KB 为单位的内存容量
        mov ah, 0x88
        int 0x15
        jc .error_hlt
        and eax,0x0000FFFF
        ; 16 位乘法，被乘数是 ax，积为 32 位。积的高 16 位在 dx 中
        ; 积的低 16 位在 ax 中
        mov cx, 0x400       ; 0x400 等于 1024,将 ax 中的内存容量换为以 byte 为单位
        mul cx
        shl edx, 16         ; 把 dx 移到高 16 位
        or edx, eax         ; 把积的低 16 位组合到 edx，为 32 位的积
        add edx, 0x100000    ; 0x88 子功能只会返回 1MB 以上的内存
                            ; 故实际内存大小要加上 1MB

        ; 输出提示信息
        mov byte [gs:0xef0],'8'
        mov byte [gs:0xef1],0x30
        mov byte [gs:0xef2],'8'
        mov byte [gs:0xef3],0x30

        jmp .mem_get_ok
; ---------------------------------------------------------------------------------

; 错误处理与储存数据容量数据
    .error_hlt:
        mov bp, error_msg   ; bp 串首地址
        mov ax, 0x40        ; AH 功能号=0x13  AL 设置写字符方式=01(显示字符串,光标跟随移动)
        mov bx, 0x2         ; BH 显示的页号=0  BL 字符属性=02h(黑底绿字)
        mov dx, 0x0200
        mov cx, e_len       ; CX 串长度=5(不包括结束符0)
        int 0x10            ; 执行BIOS 0x10 号中断
        hlt

    .mem_get_ok:
        mov [total_mem_bytes], edx
        mov bp, mem_msg		; bp 串首地址
        mov ax, 0x1301    ; AH 功能号=0x13  AL 设置写字符方式=01(显示字符串,光标跟随移动)
        mov bx, 0x30       ; BH 显示的页号=0  BL 字符属性=70h(白底黑字)
        mov dx, 0x600
        mov cx, m_len   ; CX 串长度=5(不包括结束符0)
        int 0x10          ; 执行BIOS 0x10 号中断
; ---------------------------------------------------------------------------------

; 进入保护模式
    ; 1.打开 A20 
    in al, 0x92
    or al, 0000_0010B
    out 0x92, al

    ; 2.加载gdt
    lgdt [gdt_ptr]

    ; 3.cr0 第0位置1
    mov eax, cr0
    or eax, 0x00000001
    mov cr0, eax

    ; 刷新流水线
    jmp dword SELECTOR_CODE: p_mode_start
; ---------------------------------------------------------------------------------

[bits 32]
p_mode_start:
; 选择子初始化成各段寄存器
    mov ax, SELECTOR_DATA
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, LOADER_STACK_TOP
    mov ax, SELECTOR_VIDEO
    mov gs, ax

; 输出字符串protect_msg
    mov ah, 0x31            ; 定义显示模式
    mov bx, 0x140           ; 目标内存地址的偏移量
    mov cx, p_len
    mov esi, protect_msg    ; 将要打印的字符串地址保存到esi寄存器中
    call print_msg

; 加载 kernel
   mov eax, KERNEL_START_SECTOR  ; kernel.bin 所在的扇区号
   mov ebx, KERNEL_BIN_BASE_ADDR ; 从磁盘读出后，写入到 ebx 指定的地址
   mov ecx, 200                  ; 读入的扇区数
   call rd_disk_m_32             ; 读盘

; 创建页目录及页表并初始化页内存位图
    call setup_page 
    ; gdt需要放在内核里
    ; 将描述符表地址&偏移量写入内存gdt_ptr，一会用新的地址加载
    sgdt [gdt_ptr]                          ; 取出GDT地址和偏移信息，存放在gdt_ptr这个内存位置上
    ; 视频段需要放在内核里与用户进程进行共享
    ; 将gdt描述符中视频段的段基址 + 0xc0000000
    mov ebx, [gdt_ptr + 2]                  ; 这里gdt_ptr前2字节是偏移量，后4字节是GDT基址，先选中GDT
    or dword [ebx + 0x18 + 4], 0xc0000000   ; 一个描述符8字节，0x18处是第3个段描述符也就是视频段, 修改段基址最高位为C, +4进入高4字节, 用or修改即可
    ; 将gdt的基址加上 0xc0000000 成为内核所在的地址
    add dword [gdt_ptr + 2], 0xc0000000
    add esp, 0xc0000000                     ; 将栈指针同样map到内核地址
    ; 页目录地址赋值给cr3
    mov eax, PAGE_DIR_TABLE_POS
    mov cr3, eax
    ; 打开 cr0 的 pg位 (第31位)
    mov eax, cr0
    or  eax, 0x80000000
    mov cr0, eax

; 开启分页后, 用 gdt 新的地址重新加载
    lgdt [gdt_ptr]
    
; 重新初始化gs寄存器
    mov ax, SELECTOR_VIDEO
    mov gs, ax

; 打印提示信息，表示视频段段基址已经被更新
    mov ah, 0x60
    mov bx, 0x1e0
    mov esi, v_msg
    mov cx, v_len
    call print_msg

; 刷新流水线
    jmp SELECTOR_CODE:enter_kernel	  ;强制刷新流水线,更新gdt(其实不刷新也可以)

; 分析内核
enter_kernel:
    call kernel_init
    mov esp, 0xc009f000
    jmp KERNEL_ENTRY_POINT                 ; 用地址0x1500访问测试，结果ok

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



; 将kernel.bin中的segment拷贝到编译的地址
; ---------------------------------------------------------------------------------
kernel_init:
    xor eax, eax
    xor ebx, ebx		;ebx记录程序头表地址
    xor ecx, ecx		;cx记录程序头表中的program header数量
    xor edx, edx		;dx 记录program header尺寸,即e_phentsize

    mov dx, [KERNEL_BIN_BASE_ADDR + 42]	    ; 偏移文件42字节处的属性是e_phentsize,表示program header大小
    mov ebx, [KERNEL_BIN_BASE_ADDR + 28]    ; 偏移文件开始部分28字节的地方是e_phoff,表示第1 个program header在文件中的偏移量
                                            ; 其实该值是0x34,不过还是谨慎一点，这里来读取实际值
    add ebx, KERNEL_BIN_BASE_ADDR
    mov cx, [KERNEL_BIN_BASE_ADDR + 44]     ; 偏移文件开始部分44字节的地方是e_phnum,表示有几个program header
    .each_segment:
        cmp byte [ebx + 0], PT_NULL		    ; 若p_type等于 PT_NULL,说明此program header未使用。
        je .PTNULL

        ;为函数memcpy压入参数,参数是从右往左依然压入.函数原型类似于 memcpy(dst,src,size)
        push dword [ebx + 16]           ; program header中偏移16字节的地方是p_filesz,压入函数memcpy的第三个参数:size
    mov eax, [ebx + 4]                  ; 距程序头偏移量为4字节的位置是p_offset
        add eax, KERNEL_BIN_BASE_ADDR   ; 加上kernel.bin被加载到的物理地址,eax为该段的物理地址
        push eax                        ; 压入函数memcpy的第二个参数:源地址
        push dword [ebx + 8]            ; 压入函数memcpy的第一个参数:目的地址,偏移程序头8字节的位置是p_vaddr，这就是目的地址
        call mem_cpy                    ; 调用mem_cpy完成段复制
        add esp,12                      ; 清理栈中压入的三个参数
    .PTNULL:
        add ebx, edx                    ; edx为program header大小,即e_phentsize,在此ebx指向下一个program header 
        loop .each_segment
    ret
; ---------------------------------------------------------------------------------

; 逐字节拷贝 mem_cpy(dst,src,size)
; ---------------------------------------------------------------------------------
; 输入:
    ; 栈中三个参数(dst,src,size)
; 输出:
    ; 无
mem_cpy:
    cld
    push ebp
    mov ebp, esp
    push ecx		   ; rep指令用到了ecx，但ecx对于外层段的循环还有用，故先入栈备份
    mov edi, [ebp + 8]	   ; dst
    mov esi, [ebp + 12]	   ; src
    mov ecx, [ebp + 16]	   ; size
    rep movsb		   ; 逐字节拷贝

    ;恢复环境
    pop ecx		
    pop ebp
    ret
; ---------------------------------------------------------------------------------

; 创建页目录及页表
; ---------------------------------------------------------------------------------
setup_page:
	; 先把页目录所占空间清 0
    mov ecx, 4096                               ; 1024 * 4 = 4096  
    mov esi, 0

    .clear_page_dir:
    	 mov byte [PAGE_DIR_TABLE_POS + esi], 0
         inc esi
         loop .clear_page_dir

    ; 开始创建页目录项（Page Directory Entry)
    .create_pde:
    	 mov eax, PAGE_DIR_TABLE_POS
         add eax, 0x1000                         ; 第一个页表的位置（仅次于页目录表，页目录表大小4KB）
         mov ebx, eax                            ; 0x00101 000, 储存到ebx为创建PTE时使用

         ; 下面将页目录项0和OxcOO都存为第一个页表的地址 ，每个页表表示4MB内存
         ; 这样Oxc03fffff(3G-3G04M)以下的地址和Ox003fffff(0-4M)以下的地址都 指向相同的页表
         ; 这是为将地址映射为内核地址做准备
         or eax,  PG_US_U | PG_RW_W | PG_P       ; 用户特权级，可读可写，存在内存
         mov [PAGE_DIR_TABLE_POS + 0x0], eax     ; 第一个目录项，0x00101 007
         mov [PAGE_DIR_TABLE_POS + 0xc00], eax   ; 第0xc00高10位0x300=768个页表占用的目录项，0xc00以上属于kernel空间
         ; 这里是把第768个目录页和第1个目录页指向同一个页表的物理地址：0x101000   
         ; 系统实际位于0～0x100000内存地址中，将系统虚拟地址0xc00000000映射到这低1M的空间内，只需要让0xc0000000的地址指向和低1M相同的页表即可

         sub eax, 0x1000
         mov [PAGE_DIR_TABLE_POS + 4092], eax    ; 使最后一个目录项指向页目录表自己的位置, 4092 = 1023 * 4

        
	; 创建页表项（Page Table Entry）
    mov ecx, 256                                ; 1M低端内存/每页大小4K = 256
    mov esi, 0
    mov edx, PG_US_U | PG_RW_W | PG_P           ; edx中地址为0x0，属性为7，即111b

    .create_pte:
         mov  [ebx + esi * 4], edx               ; ebx = 0x00101 000, 即第一个PTE起始地址, 每个PTE = 4 byte
         add  edx, 4096                          ; edx + 4KB
         inc  esi
         loop .create_pte                        ; 低端1M内存中，物理地址=虚拟地址，这里创建了1M空间的页表项


    ; 创建内核其他页表的PDE
    mov eax, PAGE_DIR_TABLE_POS
    add eax, 0x2000                             ; eax指向第二个页表(每个页表对应一个PDE, 含有1024个页表项)
    or  eax, PG_US_U | PG_RW_W | PG_P
    mov ebx, PAGE_DIR_TABLE_POS                 
    mov ecx, 254                                ; 769~1022的所有目录项数量, 1022 - 769 + 1 = 254
    mov esi, 769                                

    .create_kernel_pde:
         mov  [ebx + esi * 4], eax
         inc  esi
         add  eax, 0x1000                        ; eax指向下一个页表
         loop .create_kernel_pde

	ret
; ---------------------------------------------------------------------------------

; 读取硬盘函数
; --------------------------------------------------------------------------------- 
; 传入：
   ; eax 起始扇区lba地址
   ; bx loader加载到内存的地址
   ; cx 将要读入的扇区数
rd_disk_m_32:
; 0. 备份eax cx
   mov esi, eax   ; al在out指令中会使用到
   mov di, cx     ; cx在读取数据时用到

; 1. 选择通道；设置读取扇区数
   mov dx, 0x1f2  ; 设置Primary通道
   mov al, cl     ; al=cl（读入的扇区数）
   out dx, al     ; 将al的数据（读入的扇区数）写入dx指向的地址
   mov eax, esi   ; 恢复ax

; 2. 起始地址写入LBA寄存器
   ; 7-0 位地址写入端口 0x1f3
   mov dx, 0x1f3  ; 设置Primary通道
   out dx, al     ; 写入eax低8位
   ; 15-8位地址写入端口 0x1f4
   mov cl, 8      ; 设置右移位数8
   shr eax, cl    ; 将eax内地址右移8位
   mov dx, 0x1f4  ; 设置Primary通道
   out dx, al     ; 写入eax低8位(因为右移，此时低8相当于原8-15位)
   ; 23-16位地址写入端口 0x1f5
   shr eax, cl    ; 将eax内地址右移8位
   mov dx, 0x1f5  ; 设置Primary通道
   out dx, al     ; 写入eax低8位(因为右移，此时低8相当于原16-23位) 
   ; 27-24位地址写入端口 0x1f6 && 设置mba模式
   shr eax, cl    ; 将eax内地址右移8位
   and al, 0x0f   ; 保留al的低4位（此时地址就剩4位没存了）
   or al, 0xe0    ; 设置al高4位为1110（lba模式）
   mov dx, 0x1f6  ; 设置Primary通道
   out dx, al     ; 写入eax低8位(4位地址+0[DEV]+1+1[MOD]+1)

; 3. 写入读命令
   mov dx, 0x1f7  ; 设置Primary通道
   mov al, 0x20   ; 0x20读取
   out dx, al     ; 向command寄存器写入命令

; 4. 读取staus寄存器获取硬盘状态
   ; 没准备好时
   .not_ready:
      nop            ; 相当于睡眠一小会，避免太频繁查询
      in al, dx      ; 读取dx指向的寄存器status的值

      ; 第7位：1忙   第3位：1就绪
      and al, 0x88   ; 0x88=1000_1000
      cmp al, 0x08   ; 0x08=0000_1000 
                     ; ZF=1：就绪
                     ; ZF=0：忙，跳转.not_ready
   jnz .not_ready ; if ZF=0 跳转.not_ready

; 5. 读取硬盘
   mov ax, di        ; di(需要读入的扇区数)放进ax
   mov dx, 256       ; 一个扇区512字节，每次in读入2字节，共需要读取di*512/2次
   mul dx            ; 乘法 
                     ; 被乘数：ax/al
                     ; 乘数：dx
                     ; 结果：ax[8位乘法] / ax+dx[16位乘法]
   mov cx, ax        ; 将结果(循环次数)放入cx
   mov dx, 0x1f0     ; 设置Primary通道
   .go_on_read:
      in ax, dx         ; 从dx寄存器所指向的端口地址中读取2字节到ax寄存器中
      mov [ebx], ax      ; ax的数据放入bx所指向的地址中
      add ebx, 2         ; 地址向后偏移2个字节
      loop .go_on_read  ; 根据cx寄存器的值进行循环，每次循环cx--直到0

   ret               ; return 到 call 下一行
; ---------------------------------------------------------------------------------   

; 打印提示信息
; ---------------------------------------------------------------------------------
print_msg:
    lodsb               ; 从esi指向的字符串中读取一个字节到al寄存器中，并将esi自加1
    mov [gs:bx], al     ; 向目标位置写入字符
    inc bx              ; bx++
    mov [gs:bx], ah     ; 指定显示模式为ah
    inc bx              ; bx++
    dec cx              ; cx--，cx为循环次数
    cmp cx, 0           ; 判断cx是否>0
    jne print_msg       ; 循环处理，直到所有字符串都打印完毕
    
    ret
; ---------------------------------------------------------------------------------
