* NO_APP
RUNS_HUMAN_VERSION	equ	3
	.cpu 68000
	.include doscall.equ
* X68 GCC Develop
	.even
	.text
	.file	"ms.c"
	.def	_va_list
	.scl	13
	.type	18
	.endef
	.def	_size_t
	.scl	13
	.type	14
	.endef
	.def	_fpos_t
	.scl	13
	.type	4
	.endef
	.def	__stdbuf
	.scl	10
	.type	8
	.size	36
	.endef
	.def	__cnt
	.val	0
	.scl	8
	.type	4
	.endef
	.def	__bufsiz
	.val	4
	.scl	8
	.type	4
	.endef
	.def	__flag
	.val	8
	.scl	8
	.type	4
	.endef
	.def	__file
	.val	12
	.scl	8
	.type	4
	.endef
	.def	__nback
	.val	16
	.scl	8
	.type	4
	.endef
	.def	__ptr
	.val	20
	.scl	8
	.type	28
	.endef
	.def	__base
	.val	24
	.scl	8
	.type	28
	.endef
	.def	__pback
	.val	28
	.scl	8
	.dim	4
	.size	4
	.type	60
	.endef
	.def	__nbuff
	.val	32
	.scl	8
	.dim	4
	.size	4
	.type	60
	.endef
	.def	.eos
	.val	36
	.scl	102
	.tag	__stdbuf
	.size	36
	.endef
	.def	_FILE
	.scl	13
	.tag	__stdbuf
	.size	36
	.type	8
	.endef
	.def	__boxptr
	.scl	10
	.type	8
	.size	12
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_x2
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y2
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	8
	.scl	8
	.type	13
	.endef
	.def	_linestyle
	.val	10
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	12
	.scl	102
	.tag	__boxptr
	.size	12
	.endef
	.def	__circleptr
	.scl	10
	.type	8
	.size	14
	.endef
	.def	_x
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_radius
	.val	4
	.scl	8
	.type	13
	.endef
	.def	_color
	.val	6
	.scl	8
	.type	13
	.endef
	.def	_start
	.val	8
	.scl	8
	.type	3
	.endef
	.def	_end
	.val	10
	.scl	8
	.type	3
	.endef
	.def	_ratio
	.val	12
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	14
	.scl	102
	.tag	__circleptr
	.size	14
	.endef
	.def	__fillptr
	.scl	10
	.type	8
	.size	10
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_x2
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y2
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	8
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	10
	.scl	102
	.tag	__fillptr
	.size	10
	.endef
	.def	__fntbuf
	.scl	10
	.type	8
	.size	76
	.endef
	.def	_xl
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_yl
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_buffer
	.val	4
	.scl	8
	.dim	72
	.size	72
	.type	60
	.endef
	.def	.eos
	.val	76
	.scl	102
	.tag	__fntbuf
	.size	76
	.endef
	.def	__getptr
	.scl	10
	.type	8
	.size	16
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_x2
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y2
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_buf_start
	.val	8
	.scl	8
	.type	20
	.endef
	.def	_buf_end
	.val	12
	.scl	8
	.type	20
	.endef
	.def	.eos
	.val	16
	.scl	102
	.tag	__getptr
	.size	16
	.endef
	.def	__lineptr
	.scl	10
	.type	8
	.size	12
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_x2
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y2
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	8
	.scl	8
	.type	13
	.endef
	.def	_linestyle
	.val	10
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	12
	.scl	102
	.tag	__lineptr
	.size	12
	.endef
	.def	__paintptr
	.scl	10
	.type	8
	.size	14
	.endef
	.def	_x
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	4
	.scl	8
	.type	13
	.endef
	.def	_buf_start
	.val	6
	.scl	8
	.type	20
	.endef
	.def	_buf_end
	.val	10
	.scl	8
	.type	20
	.endef
	.def	.eos
	.val	14
	.scl	102
	.tag	__paintptr
	.size	14
	.endef
	.def	__pointptr
	.scl	10
	.type	8
	.size	6
	.endef
	.def	_x
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	4
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	6
	.scl	102
	.tag	__pointptr
	.size	6
	.endef
	.def	__psetptr
	.scl	10
	.type	8
	.size	6
	.endef
	.def	_x
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_color
	.val	4
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	6
	.scl	102
	.tag	__psetptr
	.size	6
	.endef
	.def	__putptr
	.scl	10
	.type	8
	.size	16
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_x2
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y2
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_buf_start
	.val	8
	.scl	8
	.type	20
	.endef
	.def	_buf_end
	.val	12
	.scl	8
	.type	20
	.endef
	.def	.eos
	.val	16
	.scl	102
	.tag	__putptr
	.size	16
	.endef
	.def	__symbolptr
	.scl	10
	.type	8
	.size	14
	.endef
	.def	_x1
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_string_address
	.val	4
	.scl	8
	.type	28
	.endef
	.def	_mag_x
	.val	8
	.scl	8
	.type	12
	.endef
	.def	_mag_y
	.val	9
	.scl	8
	.type	12
	.endef
	.def	_color
	.val	10
	.scl	8
	.type	13
	.endef
	.def	_font_type
	.val	12
	.scl	8
	.type	12
	.endef
	.def	_angle
	.val	13
	.scl	8
	.type	12
	.endef
	.def	.eos
	.val	14
	.scl	102
	.tag	__symbolptr
	.size	14
	.endef
	.def	__regs
	.scl	10
	.type	8
	.size	56
	.endef
	.def	_d0
	.val	0
	.scl	8
	.type	4
	.endef
	.def	_d1
	.val	4
	.scl	8
	.type	4
	.endef
	.def	_d2
	.val	8
	.scl	8
	.type	4
	.endef
	.def	_d3
	.val	12
	.scl	8
	.type	4
	.endef
	.def	_d4
	.val	16
	.scl	8
	.type	4
	.endef
	.def	_d5
	.val	20
	.scl	8
	.type	4
	.endef
	.def	_d6
	.val	24
	.scl	8
	.type	4
	.endef
	.def	_d7
	.val	28
	.scl	8
	.type	4
	.endef
	.def	_a1
	.val	32
	.scl	8
	.type	4
	.endef
	.def	_a2
	.val	36
	.scl	8
	.type	4
	.endef
	.def	_a3
	.val	40
	.scl	8
	.type	4
	.endef
	.def	_a4
	.val	44
	.scl	8
	.type	4
	.endef
	.def	_a5
	.val	48
	.scl	8
	.type	4
	.endef
	.def	_a6
	.val	52
	.scl	8
	.type	4
	.endef
	.def	.eos
	.val	56
	.scl	102
	.tag	__regs
	.size	56
	.endef
	.def	__chain
	.scl	10
	.type	8
	.size	6
	.endef
	.def	_addr
	.val	0
	.scl	8
	.type	20
	.endef
	.def	_len
	.val	4
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	6
	.scl	102
	.tag	__chain
	.size	6
	.endef
	.def	__chain2
	.scl	10
	.type	8
	.size	10
	.endef
	.def	_addr
	.val	0
	.scl	8
	.type	20
	.endef
	.def	_len
	.val	4
	.scl	8
	.type	13
	.endef
	.def	_next
	.val	6
	.scl	8
	.tag	__chain2
	.size	10
	.type	24
	.endef
	.def	.eos
	.val	10
	.scl	102
	.tag	__chain2
	.size	10
	.endef
	.def	__clipxy
	.scl	10
	.type	8
	.size	8
	.endef
	.def	_xs
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_ys
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_xe
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_ye
	.val	6
	.scl	8
	.type	3
	.endef
	.def	.eos
	.val	8
	.scl	102
	.tag	__clipxy
	.size	8
	.endef
	.def	__patst
	.scl	10
	.type	8
	.size	68
	.endef
	.def	_offsetx
	.val	0
	.scl	8
	.type	3
	.endef
	.def	_offsety
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_shadow
	.val	4
	.scl	8
	.dim	16
	.size	32
	.type	51
	.endef
	.def	_pattern
	.val	36
	.scl	8
	.dim	16
	.size	32
	.type	51
	.endef
	.def	.eos
	.val	68
	.scl	102
	.tag	__patst
	.size	68
	.endef
	.def	__tboxptr
	.scl	10
	.type	8
	.size	12
	.endef
	.def	_vram_page
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_x
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_x1
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	8
	.scl	8
	.type	3
	.endef
	.def	_line_style
	.val	10
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	12
	.scl	102
	.tag	__tboxptr
	.size	12
	.endef
	.def	__txfillptr
	.scl	10
	.type	8
	.size	12
	.endef
	.def	_vram_page
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_x
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_x1
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	8
	.scl	8
	.type	3
	.endef
	.def	_fill_patn
	.val	10
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	12
	.scl	102
	.tag	__txfillptr
	.size	12
	.endef
	.def	__trevptr
	.scl	10
	.type	8
	.size	10
	.endef
	.def	_vram_page
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_x
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_x1
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	8
	.scl	8
	.type	3
	.endef
	.def	.eos
	.val	10
	.scl	102
	.tag	__trevptr
	.size	10
	.endef
	.def	__xlineptr
	.scl	10
	.type	8
	.size	10
	.endef
	.def	_vram_page
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_x
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_x1
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_line_style
	.val	8
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	10
	.scl	102
	.tag	__xlineptr
	.size	10
	.endef
	.def	__ylineptr
	.scl	10
	.type	8
	.size	10
	.endef
	.def	_vram_page
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_x
	.val	2
	.scl	8
	.type	3
	.endef
	.def	_y
	.val	4
	.scl	8
	.type	3
	.endef
	.def	_y1
	.val	6
	.scl	8
	.type	3
	.endef
	.def	_line_style
	.val	8
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	10
	.scl	102
	.tag	__ylineptr
	.size	10
	.endef
	.def	_dosmode_t
	.scl	13
	.type	4
	.endef
	.def	_devmode_t
	.scl	13
	.type	4
	.endef
	.def	__inpptr
	.scl	10
	.type	8
	.size	258
	.endef
	.def	_max
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_length
	.val	1
	.scl	8
	.type	12
	.endef
	.def	_buffer
	.val	2
	.scl	8
	.dim	256
	.size	256
	.type	50
	.endef
	.def	.eos
	.val	258
	.scl	102
	.tag	__inpptr
	.size	258
	.endef
	.def	__nameckbuf
	.scl	10
	.type	8
	.size	92
	.endef
	.def	_drive
	.val	0
	.scl	8
	.dim	2
	.size	2
	.type	50
	.endef
	.def	_path
	.val	2
	.scl	8
	.dim	65
	.size	65
	.type	50
	.endef
	.def	_name
	.val	67
	.scl	8
	.dim	19
	.size	19
	.type	50
	.endef
	.def	_ext
	.val	86
	.scl	8
	.dim	5
	.size	5
	.type	50
	.endef
	.def	.eos
	.val	92
	.scl	102
	.tag	__nameckbuf
	.size	92
	.endef
	.def	_.fake0
	.scl	10
	.type	8
	.size	96
	.endef
	.def	_dupcnt
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_devattr
	.val	1
	.scl	8
	.type	12
	.endef
	.def	_deventry
	.val	2
	.scl	8
	.type	20
	.endef
	.def	_nouse_1
	.val	6
	.scl	8
	.dim	8
	.size	8
	.type	50
	.endef
	.def	_openmode
	.val	14
	.scl	8
	.type	12
	.endef
	.def	_nouse_2
	.val	15
	.scl	8
	.dim	21
	.size	21
	.type	50
	.endef
	.def	_name1
	.val	36
	.scl	8
	.dim	8
	.size	8
	.type	50
	.endef
	.def	_ext
	.val	44
	.scl	8
	.dim	3
	.size	3
	.type	50
	.endef
	.def	_nouse_3
	.val	47
	.scl	8
	.type	2
	.endef
	.def	_name2
	.val	48
	.scl	8
	.dim	10
	.size	10
	.type	50
	.endef
	.def	_nouse_4
	.val	58
	.scl	8
	.dim	38
	.size	38
	.type	50
	.endef
	.def	.eos
	.val	96
	.scl	102
	.tag	_.fake0
	.size	96
	.endef
	.def	_.fake1
	.scl	10
	.type	8
	.size	96
	.endef
	.def	_dupcnt
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_mode
	.val	8
	.scl	18
	.type	4
	.size	1
	.endef
	.def	_unknown
	.val	9
	.scl	18
	.type	4
	.size	2
	.endef
	.def	_physdrv
	.val	11
	.scl	18
	.type	4
	.size	5
	.endef
	.def	_deventry
	.val	2
	.scl	8
	.type	20
	.endef
	.def	_fileptr
	.val	6
	.scl	8
	.type	14
	.endef
	.def	_exclptr
	.val	10
	.scl	8
	.type	14
	.endef
	.def	_openmode
	.val	14
	.scl	8
	.type	12
	.endef
	.def	_entryidx
	.val	15
	.scl	8
	.type	12
	.endef
	.def	_clustidx
	.val	16
	.scl	8
	.type	12
	.endef
	.def	_nouse_2
	.val	17
	.scl	8
	.type	2
	.endef
	.def	_acluster
	.val	18
	.scl	8
	.type	13
	.endef
	.def	_asector
	.val	20
	.scl	8
	.type	14
	.endef
	.def	_iobuf
	.val	24
	.scl	8
	.type	20
	.endef
	.def	_dirsec
	.val	28
	.scl	8
	.type	14
	.endef
	.def	_fptrmax
	.val	32
	.scl	8
	.type	14
	.endef
	.def	_name1
	.val	36
	.scl	8
	.dim	8
	.size	8
	.type	50
	.endef
	.def	_ext
	.val	44
	.scl	8
	.dim	3
	.size	3
	.type	50
	.endef
	.def	_attr
	.val	47
	.scl	8
	.type	12
	.endef
	.def	_name2
	.val	48
	.scl	8
	.dim	10
	.size	10
	.type	50
	.endef
	.def	_time
	.val	58
	.scl	8
	.type	13
	.endef
	.def	_date
	.val	60
	.scl	8
	.type	13
	.endef
	.def	_fatno
	.val	62
	.scl	8
	.type	13
	.endef
	.def	_size
	.val	64
	.scl	8
	.type	14
	.endef
	.def	_nouse_4
	.val	68
	.scl	8
	.dim	28
	.size	28
	.type	50
	.endef
	.def	.eos
	.val	96
	.scl	102
	.tag	_.fake1
	.size	96
	.endef
	.def	__fcb
	.scl	12
	.type	9
	.size	96
	.endef
	.def	_chr
	.val	0
	.scl	11
	.tag	_.fake0
	.size	96
	.type	8
	.endef
	.def	_blk
	.val	0
	.scl	11
	.tag	_.fake1
	.size	96
	.type	8
	.endef
	.def	.eos
	.val	96
	.scl	102
	.tag	__fcb
	.size	96
	.endef
	.def	__indos
	.scl	10
	.type	8
	.size	14
	.endef
	.def	_indosf
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_doscmd
	.val	2
	.scl	8
	.type	12
	.endef
	.def	_fat_flg
	.val	3
	.scl	8
	.type	12
	.endef
	.def	_retry_count
	.val	4
	.scl	8
	.type	13
	.endef
	.def	_retry_time
	.val	6
	.scl	8
	.type	13
	.endef
	.def	_verifyf
	.val	8
	.scl	8
	.type	13
	.endef
	.def	_breakf
	.val	10
	.scl	8
	.type	12
	.endef
	.def	_ctrlpf
	.val	11
	.scl	8
	.type	12
	.endef
	.def	_reserved
	.val	12
	.scl	8
	.type	12
	.endef
	.def	_wkcurdrv
	.val	13
	.scl	8
	.type	12
	.endef
	.def	.eos
	.val	14
	.scl	102
	.tag	__indos
	.size	14
	.endef
	.def	__mep
	.scl	10
	.type	8
	.size	16
	.endef
	.def	_prev_mp
	.val	0
	.scl	8
	.type	20
	.endef
	.def	_parent_mp
	.val	4
	.scl	8
	.type	20
	.endef
	.def	_block_end
	.val	8
	.scl	8
	.type	20
	.endef
	.def	_next_mp
	.val	12
	.scl	8
	.type	20
	.endef
	.def	.eos
	.val	16
	.scl	102
	.tag	__mep
	.size	16
	.endef
	.def	__psp
	.scl	10
	.type	8
	.size	240
	.endef
	.def	_env
	.val	0
	.scl	8
	.type	18
	.endef
	.def	_exit
	.val	4
	.scl	8
	.type	20
	.endef
	.def	_ctrlc
	.val	8
	.scl	8
	.type	20
	.endef
	.def	_errexit
	.val	12
	.scl	8
	.type	20
	.endef
	.def	_comline
	.val	16
	.scl	8
	.type	18
	.endef
	.def	_handle
	.val	20
	.scl	8
	.dim	12
	.size	12
	.type	60
	.endef
	.def	_bss
	.val	32
	.scl	8
	.type	20
	.endef
	.def	_heap
	.val	36
	.scl	8
	.type	20
	.endef
	.def	_stack
	.val	40
	.scl	8
	.type	20
	.endef
	.def	_usp
	.val	44
	.scl	8
	.type	20
	.endef
	.def	_ssp
	.val	48
	.scl	8
	.type	20
	.endef
	.def	_sr
	.val	52
	.scl	8
	.type	13
	.endef
	.def	_abort_sr
	.val	54
	.scl	8
	.type	13
	.endef
	.def	_abort_ssp
	.val	56
	.scl	8
	.type	20
	.endef
	.def	_trap10
	.val	60
	.scl	8
	.type	20
	.endef
	.def	_trap11
	.val	64
	.scl	8
	.type	20
	.endef
	.def	_trap12
	.val	68
	.scl	8
	.type	20
	.endef
	.def	_trap13
	.val	72
	.scl	8
	.type	20
	.endef
	.def	_trap14
	.val	76
	.scl	8
	.type	20
	.endef
	.def	_osflg
	.val	80
	.scl	8
	.type	14
	.endef
	.def	_reserve_1
	.val	84
	.scl	8
	.dim	28
	.size	28
	.type	60
	.endef
	.def	_exe_path
	.val	112
	.scl	8
	.dim	68
	.size	68
	.type	50
	.endef
	.def	_exe_name
	.val	180
	.scl	8
	.dim	24
	.size	24
	.type	50
	.endef
	.def	_reserve_2
	.val	204
	.scl	8
	.dim	36
	.size	36
	.type	50
	.endef
	.def	.eos
	.val	240
	.scl	102
	.tag	__psp
	.size	240
	.endef
	.def	__comline
	.scl	10
	.type	8
	.size	256
	.endef
	.def	_len
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_buffer
	.val	1
	.scl	8
	.dim	255
	.size	255
	.type	50
	.endef
	.def	.eos
	.val	256
	.scl	102
	.tag	__comline
	.size	256
	.endef
	.def	__namestbuf
	.scl	10
	.type	8
	.size	88
	.endef
	.def	_flg
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_drive
	.val	1
	.scl	8
	.type	12
	.endef
	.def	_path
	.val	2
	.scl	8
	.dim	65
	.size	65
	.type	50
	.endef
	.def	_name1
	.val	67
	.scl	8
	.dim	8
	.size	8
	.type	50
	.endef
	.def	_ext
	.val	75
	.scl	8
	.dim	3
	.size	3
	.type	50
	.endef
	.def	_name2
	.val	78
	.scl	8
	.dim	10
	.size	10
	.type	50
	.endef
	.def	.eos
	.val	88
	.scl	102
	.tag	__namestbuf
	.size	88
	.endef
	.def	__freeinf
	.scl	10
	.type	8
	.size	8
	.endef
	.def	_free
	.val	0
	.scl	8
	.type	13
	.endef
	.def	_max
	.val	2
	.scl	8
	.type	13
	.endef
	.def	_sec
	.val	4
	.scl	8
	.type	13
	.endef
	.def	_byte
	.val	6
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	8
	.scl	102
	.tag	__freeinf
	.size	8
	.endef
	.def	__dpbptr
	.scl	10
	.type	8
	.size	94
	.endef
	.def	_drive
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_unit
	.val	1
	.scl	8
	.type	12
	.endef
	.def	_byte
	.val	2
	.scl	8
	.type	13
	.endef
	.def	_sec
	.val	4
	.scl	8
	.type	12
	.endef
	.def	_shift
	.val	5
	.scl	8
	.type	12
	.endef
	.def	_fatsec
	.val	6
	.scl	8
	.type	13
	.endef
	.def	_fatcount
	.val	8
	.scl	8
	.type	12
	.endef
	.def	_fatlen
	.val	9
	.scl	8
	.type	12
	.endef
	.def	_dircount
	.val	10
	.scl	8
	.type	13
	.endef
	.def	_datasec
	.val	12
	.scl	8
	.type	13
	.endef
	.def	_maxfat
	.val	14
	.scl	8
	.type	13
	.endef
	.def	_dirsec
	.val	16
	.scl	8
	.type	13
	.endef
	.def	_driver
	.val	18
	.scl	8
	.type	4
	.endef
	.def	_ide
	.val	22
	.scl	8
	.type	12
	.endef
	.def	_flg
	.val	23
	.scl	8
	.type	12
	.endef
	.def	_next
	.val	24
	.scl	8
	.tag	__dpbptr
	.size	94
	.type	24
	.endef
	.def	_dirfat
	.val	28
	.scl	8
	.type	13
	.endef
	.def	_dirbuf
	.val	30
	.scl	8
	.dim	64
	.size	64
	.type	50
	.endef
	.def	.eos
	.val	94
	.scl	102
	.tag	__dpbptr
	.size	94
	.endef
	.def	__filbuf
	.scl	10
	.type	8
	.size	54
	.endef
	.def	_searchatr
	.val	0
	.scl	8
	.type	12
	.endef
	.def	_driveno
	.val	1
	.scl	8
	.type	12
	.endef
	.def	_dirsec
	.val	2
	.scl	8
	.type	14
	.endef
	.def	_dirlft
	.val	6
	.scl	8
	.type	13
	.endef
	.def	_dirpos
	.val	8
	.scl	8
	.type	13
	.endef
	.def	_filename
	.val	10
	.scl	8
	.dim	8
	.size	8
	.type	50
	.endef
	.def	_ext
	.val	18
	.scl	8
	.dim	3
	.size	3
	.type	50
	.endef
	.def	_atr
	.val	21
	.scl	8
	.type	12
	.endef
	.def	_time
	.val	22
	.scl	8
	.type	13
	.endef
	.def	_date
	.val	24
	.scl	8
	.type	13
	.endef
	.def	_filelen
	.val	26
	.scl	8
	.type	14
	.endef
	.def	_name
	.val	30
	.scl	8
	.dim	23
	.size	23
	.type	50
	.endef
	.def	.eos
	.val	54
	.scl	102
	.tag	__filbuf
	.size	54
	.endef
	.def	__dregs
	.scl	10
	.type	8
	.size	60
	.endef
	.def	_d0
	.val	0
	.scl	8
	.type	4
	.endef
	.def	_d1
	.val	4
	.scl	8
	.type	4
	.endef
	.def	_d2
	.val	8
	.scl	8
	.type	4
	.endef
	.def	_d3
	.val	12
	.scl	8
	.type	4
	.endef
	.def	_d4
	.val	16
	.scl	8
	.type	4
	.endef
	.def	_d5
	.val	20
	.scl	8
	.type	4
	.endef
	.def	_d6
	.val	24
	.scl	8
	.type	4
	.endef
	.def	_d7
	.val	28
	.scl	8
	.type	4
	.endef
	.def	_a0
	.val	32
	.scl	8
	.type	4
	.endef
	.def	_a1
	.val	36
	.scl	8
	.type	4
	.endef
	.def	_a2
	.val	40
	.scl	8
	.type	4
	.endef
	.def	_a3
	.val	44
	.scl	8
	.type	4
	.endef
	.def	_a4
	.val	48
	.scl	8
	.type	4
	.endef
	.def	_a5
	.val	52
	.scl	8
	.type	4
	.endef
	.def	_a6
	.val	56
	.scl	8
	.type	4
	.endef
	.def	.eos
	.val	60
	.scl	102
	.tag	__dregs
	.size	60
	.endef
	.def	__prcctrl
	.scl	10
	.type	8
	.size	12
	.endef
	.def	_length
	.val	0
	.scl	8
	.type	4
	.endef
	.def	_buf_ptr
	.val	4
	.scl	8
	.type	28
	.endef
	.def	_command
	.val	8
	.scl	8
	.type	13
	.endef
	.def	_your_id
	.val	10
	.scl	8
	.type	13
	.endef
	.def	.eos
	.val	12
	.scl	102
	.tag	__prcctrl
	.size	12
	.endef
	.def	__prcptr
	.scl	10
	.type	8
	.size	116
	.endef
	.def	_next_ptr
	.val	0
	.scl	8
	.tag	__prcptr
	.size	116
	.type	24
	.endef
	.def	_wait_flg
	.val	4
	.scl	8
	.type	12
	.endef
	.def	_counter
	.val	5
	.scl	8
	.type	12
	.endef
	.def	_max_counter
	.val	6
	.scl	8
	.type	12
	.endef
	.def	_doscmd
	.val	7
	.scl	8
	.type	12
	.endef
	.def	_psp_id
	.val	8
	.scl	8
	.type	14
	.endef
	.def	_usp_reg
	.val	12
	.scl	8
	.type	14
	.endef
	.def	_d_reg
	.val	16
	.scl	8
	.dim	8
	.size	32
	.type	62
	.endef
	.def	_a_reg
	.val	48
	.scl	8
	.dim	7
	.size	28
	.type	62
	.endef
	.def	_sr_reg
	.val	76
	.scl	8
	.type	13
	.endef
	.def	_pc_reg
	.val	78
	.scl	8
	.type	14
	.endef
	.def	_ssp_reg
	.val	82
	.scl	8
	.type	14
	.endef
	.def	_indosf
	.val	86
	.scl	8
	.type	13
	.endef
	.def	_indosp
	.val	88
	.scl	8
	.type	14
	.endef
	.def	_buf_ptr
	.val	92
	.scl	8
	.tag	__prcctrl
	.size	12
	.type	24
	.endef
	.def	_name
	.val	96
	.scl	8
	.dim	16
	.size	16
	.type	60
	.endef
	.def	_wait_time
	.val	112
	.scl	8
	.type	4
	.endef
	.def	.eos
	.val	116
	.scl	102
	.tag	__prcptr
	.size	116
	.endef
	.def	_MMem
	.val	_MMem
	.scl	2
	.type	20
	.endef
	.def	_VRAM
	.val	_VRAM
	.scl	2
	.type	20
	.endef
	.def	_fontfile
	.val	_fontfile
	.scl	2
	.dim	9
	.size	9
	.type	50
	.endef
	.globl _fontfile
	.even
	.data
_fontfile:
	.dc.b 'font.fon',$00
	.even
	.text
	.def	_command
	.val	_command
	.scl	2
	.dim	256
	.size	256
	.type	50
	.endef
	.globl _command
	.even
	.data
_command:
	.dc.b $00
	.ds.b 255
	.even
	.text
	.def	_commands
	.val	_commands
	.scl	2
	.dim	64
	.size	256
	.type	628
	.endef
	.def	_command_name
	.val	_command_name
	.scl	2
	.dim	64,10
	.size	640
	.type	242
	.endef
	.globl _command_name
	.even
	.data
_command_name:
	.dc.b 'ASSIGN',$00
	.ds.b 3
	.dc.b 'ATDIR',$00
	.ds.b 4
	.dc.b 'ATTRIB',$00
	.ds.b 3
	.dc.b 'BASIC',$00
	.ds.b 4
	.dc.b 'BUFFERS',$00
	.ds.b 2
	.dc.b 'CD',$00
	.ds.b 7
	.dc.b 'CHDIR',$00
	.ds.b 4
	.dc.b 'CHKDSK',$00
	.ds.b 3
	.dc.b 'CLS',$00
	.ds.b 6
	.dc.b 'COMMAND2',$00
	.ds.b 1
	.dc.b 'CONCAT',$00
	.ds.b 3
	.dc.b 'COPY',$00
	.ds.b 5
	.dc.b 'DATE',$00
	.ds.b 5
	.dc.b 'DEL',$00
	.ds.b 6
	.dc.b 'DIR',$00
	.ds.b 6
	.dc.b 'ECHO',$00
	.ds.b 5
	.dc.b 'ERA',$00
	.ds.b 6
	.dc.b 'ERASE',$00
	.ds.b 4
	.dc.b 'EXIT',$00
	.ds.b 5
	.dc.b 'FORMAT',$00
	.ds.b 3
	.dc.b 'HELP',$00
	.ds.b 5
	.dc.b 'IF',$00
	.ds.b 7
	.dc.b 'MD',$00
	.ds.b 7
	.dc.b 'MKDIR',$00
	.ds.b 4
	.dc.b 'MODE',$00
	.ds.b 5
	.dc.b 'MOVE',$00
	.ds.b 5
	.dc.b 'MVDIR',$00
	.ds.b 4
	.dc.b 'PATH',$00
	.ds.b 5
	.dc.b 'PAUSE',$00
	.ds.b 4
	.dc.b 'RAMDISK',$00
	.ds.b 2
	.dc.b 'RD',$00
	.ds.b 7
	.dc.b 'RD',$00
	.ds.b 7
	.dc.b 'REM',$00
	.ds.b 6
	.dc.b 'REN',$00
	.ds.b 6
	.dc.b 'RENAME',$00
	.ds.b 3
	.dc.b 'RMDIR',$00
	.ds.b 4
	.dc.b 'RNDIR',$00
	.ds.b 4
	.dc.b 'SET',$00
	.ds.b 6
	.dc.b 'TIME',$00
	.ds.b 5
	.dc.b 'TYPE',$00
	.ds.b 5
	.dc.b 'UNDEL',$00
	.ds.b 4
	.dc.b 'VER',$00
	.ds.b 6
	.dc.b 'VERIFY',$00
	.ds.b 3
	.dc.b 'VOL',$00
	.ds.b 6
	.ds.b 200
	.even
	.text
	.def	_com
	.val	_com
	.scl	2
	.type	4
	.endef
	.def	_ssp
	.val	_ssp
	.scl	2
	.type	4
	.endef
	.def	_screen_mode
	.val	_screen_mode
	.scl	2
	.type	4
	.endef
	.def	_width_size
	.val	_width_size
	.scl	2
	.type	4
	.endef
	.def	_do_command
	.val	_do_command
	.scl	2
	.type	36
	.endef
	.even
	.globl _do_command
_do_command:
	link a6,#-512
	movem.l d3/d4/d5/a3,-(sp)
	.def	.bf
	.val	.
	.scl	101
	.line	42
	.endef
	.def	_command
	.val	8
	.scl	9
	.type	18
	.endef
	.def	_command
	.val	9
	.scl	4
	.type	18
	.endef
	move.l 8(a6),a1
	.def	.bb
	.val	.
	.scl	100
	.line	1
	.endef
	.def	_i
	.val	3
	.scl	4
	.type	4
	.endef
	.def	_j
	.val	5
	.scl	4
	.type	4
	.endef
	.def	_arguments
	.val	-512
	.scl	1
	.dim	128
	.size	512
	.type	114
	.endef
	.ln	5
	moveq.l #0,d3
	.ln	6
	move.l d3,d5
	.ln	7
	jbra ?2
?4:
	.ln	8
	addq.l #1,d3
?2:
	cmp.b #32,(a1,d3.l)
	jbeq ?4
	.ln	10
	move.l d5,d0
	asl.l #2,d0
	lea (a6,d0.l),a0
	move.l a1,d1
	add.l d3,d1
	move.l d1,-512(a0)
	addq.l #1,d5
	.ln	11
	jbra ?5
?12:
	.ln	12
	cmp.b #32,(a1,d3.l)
	jbne ?7
	.ln	13
	clr.b (a1,d3.l)
	.ln	14
?11:
	.ln	15
	addq.l #1,d3
	cmp.b #32,(a1,d3.l)
	jbeq ?11
	.ln	17
	move.l d5,d0
	asl.l #2,d0
	lea (a6,d0.l),a0
	move.l a1,d1
	add.l d3,d1
	move.l d1,-512(a0)
	addq.l #1,d5
	.ln	11
?7:
	addq.l #1,d3
?5:
	cmp.l _com,d3
	jblt ?12
	.ln	20
	move.l d5,d0
	asl.l #2,d0
	lea (a6,d0.l),a0
	moveq.l #0,d1
	move.l d1,-512(a0)
	.ln	22
	move.l -512(a6),a0
	tst.b (a0)
	jbeq ?1
	.ln	23
	.ln	25
	moveq.l #0,d3
	lea _commands,a3
?18:
	.ln	26
	move.l d3,d4
	asl.l #2,d4
	move.l d4,d0
	add.l d3,d0
	add.l d0,d0
	add.l #_command_name,d0
	move.l d0,-(sp)
	move.l -512(a6),-(sp)
	jbsr _stricmp
	addq.w #8,sp
	tst.l d0
	jbne ?16
	.ln	27
	pea -512(a6)
	move.l d5,-(sp)
	move.l (a3,d4.l),a0
	jbsr (a0)
	.ln	28
	jbra ?1
	.ln	25
?16:
	addq.l #1,d3
	moveq.l #63,d1
	cmp.l d3,d1
	jbge ?18
	.ln	31
	jbsr _nothing
	.def	.eb
	.val	.
	.scl	100
	.line	31
	.endef
	.ln	32
?1:
	.def	.ef
	.val	.
	.scl	101
	.line	32
	.endef
	movem.l -528(a6),d3/d4/d5/a3
	unlk a6
	rts
	.def	_do_command
	.val	.
	.scl	-1
	.endef
	.def	_screen
	.val	_screen
	.scl	2
	.type	36
	.endef
	.even
	.globl _screen
_screen:
	link a6,#0
	move.l d3,-(sp)
	.def	.bf
	.val	.
	.scl	101
	.line	75
	.endef
	.def	_mode
	.val	8
	.scl	9
	.type	4
	.endef
	.def	_mode
	.val	3
	.scl	4
	.type	4
	.endef
	move.l 8(a6),d3
	.ln	2
	move.l d3,-(sp)
	jbsr _set_screen
	.ln	3
	move.l d3,_screen_mode
	.ln	4
	.def	.ef
	.val	.
	.scl	101
	.line	4
	.endef
	move.l -4(a6),d3
	unlk a6
	rts
	.def	_screen
	.val	.
	.scl	-1
	.endef
	.def	_width
	.val	_width
	.scl	2
	.type	36
	.endef
	.even
	.globl _width
_width:
	link a6,#0
	move.l d3,-(sp)
	.def	.bf
	.val	.
	.scl	101
	.line	80
	.endef
	.def	_size
	.val	8
	.scl	9
	.type	4
	.endef
	.def	_size
	.val	3
	.scl	4
	.type	4
	.endef
	move.l 8(a6),d3
	.ln	2
	move.l d3,-(sp)
	jbsr _set_width
	.ln	3
	move.l d3,_width_size
	.ln	4
	.def	.ef
	.val	.
	.scl	101
	.line	4
	.endef
	move.l -4(a6),d3
	unlk a6
	rts
	.def	_width
	.val	.
	.scl	-1
	.endef
?C0:
	.dc.b '0',$00
?C1:
	.dc.b '0123456789ABCDEF',$00
?C2:
	.dc.b 'ÉÅÉÇÉäÇ™ämï€Ç≈Ç´Ç‹ÇπÇÒ',$0a,$00
	.def	_main
	.val	_main
	.scl	2
	.type	36
	.endef
	.even
	.xref __main
	.xdef _main
_main:
	link a6,#-20
	move.l a3,-(sp)
	.def	.bf
	.val	.
	.scl	101
	.line	86
	.endef
	.def	.bb
	.val	.
	.scl	100
	.line	1
	.endef
	.def	_sym
	.val	-2
	.scl	1
	.dim	2
	.size	2
	.type	60
	.endef
	.def	_hex
	.val	-20
	.scl	1
	.dim	17
	.size	17
	.type	60
	.endef
	.ln	4
	lea ?C0(OPC),a0
	move.b (a0)+,-2(a6)
	move.b (a0),-1(a6)
	pea 0.w
	pea 0.w
	move.l a6,-(sp)
	lea _memset,a3
	jbsr (a3)
	lea 12(sp),sp
	.ln	5
	pea 17.w
	pea ?C1(OPC)
	pea -20(a6)
	jbsr _memcpy
	lea 12(sp),sp
	pea 0.w
	pea 0.w
	pea -3(a6)
	jbsr (a3)
	lea 12(sp),sp
	.ln	7
	move.l #65536,-(sp)
	jbsr __dos_malloc
	move.l d0,_MMem
	.ln	8
	addq.w #4,sp
	cmp.l #-2130706433,_MMem
	jbls ?22
	.ln	9
	pea ?C2(OPC)
	jbsr _printf
	.ln	10
	jbra ?21
?22:
	.ln	13
	pea _fontfile
	jbsr _vdp_init
	move.l d0,_VRAM
	.ln	15
	pea 0.w
	jbsr __iocs_b_super
	move.l d0,_ssp
	.ln	17
	move.l _VRAM,-(sp)
	move.l _MMem,-(sp)
	jbsr _login
	.ln	19
	jbsr _display_set
	.ln	21
	pea 0.w
	jbsr _screen
	.ln	23
	pea 80.w
	jbsr _width
	.ln	25
	jbsr _command_ms
	.def	.eb
	.val	.
	.scl	100
	.line	25
	.endef
	.ln	27
?21:
	.def	.ef
	.val	.
	.scl	101
	.line	27
	.endef
	move.l -24(a6),a3
	unlk a6
	rts
	.def	_main
	.val	.
	.scl	-1
	.endef
	.even
	.bss
	.xdef _width_size
	.comm _width_size,4
	.xdef _screen_mode
	.comm _screen_mode,4
	.xdef _ssp
	.comm _ssp,4
	.xdef _com
	.comm _com,4
	.xdef _commands
	.comm _commands,256
	.xdef _VRAM
	.comm _VRAM,4
	.xdef _MMem
	.comm _MMem,4
	.even
	.text
	.even
	.end
