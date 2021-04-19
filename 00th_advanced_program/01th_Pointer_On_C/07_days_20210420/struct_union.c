/**
 *	1、定义与申明的区别：
 *		定义包含了申明，定义是从无到有，申明是告诉编译器，有！
 *		比如int i = 0;这里就是定义了一个变量i，并且申明了有变量i；
 *		extern int i = 0;这里就只是申明有变量i，具体在哪定义就只有编译器自己去找了！
 *		但是在日常使用中，有时候直接说申明也没有错，毕竟我们在定义一个变量或者一个函数
 *		的时候，实际也申明了这个变量或者函数。 
 *
 *	2、结构变量本身不同于数组，数组名可以是数组第一个元素的地址，是一个指针，
 *		但是结构变量不同，结构变量是一个标量类型。
 *
 *	3、
 *
 */

/*define a struct variable value*/
static struct tag_name {

	char member_list;

} variable_list1;

struct tag_name variable_list2;

static typedef struct {
	char member_list;
} tag_type;


tag_type variable_list3;
