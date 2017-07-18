# `hive`

hive是一个简单的LUA应用框架,目前基于LUA 5.3.4.  
主要提供了文件沙盒,文件热加载以及一些基础的服务程序底层支持.  

## 编译环境

luna同时支持Windows, Linux, MacOS三平台,编译器必须支持C++14.  

- Windows: Visual studio 2015以上版本,需要自行编译lua的dll库,或者用win64目录下的库文件.
- MacOS: 需要自行编译安装lua.
- Linux: 需要自行编译安装lua.

## 文件沙盒及文件热加载

在hive中,用户主要通过import函数加载lua文件.
- import为每个文件创建了一个沙盒环境,这使得各个文件中的变量名不会冲突.
- 通过import加载文件时,可以通过环境变量设置根路径,如: `LUA_ROOT=/data/script/`
- 在需要的地方,用户也可以同时使用require加载所需文件.
- 多次import同一个文件,实际上只会加重一次,import返回的是文件的环境表.
- 文件时间戳变化是,会自动重新加载.

## 启停与信号

程序启动后,首先加载入口文件.  
如果程序需要持续执行,而不是仅仅执行一遍,那么在hive上定义一个名为'run'的函数.   
这样hive框架会一直循环的调用`hive.run`,如果程序想要退出,那么简单的将`hive.run`赋值为nil即可.
在循环调用的过程中,如果发现入口文件时间戳变化,会自动重新加载.   
用户可以注册信号`hive.register_signal(signo)`,注册后,可以通过掩码`hive.signal`来检视.  
注意,用户需要在'hive.run'中调用sleep,否则会出现CPU完全占用的情况.  

```lua
count = count or 0;
function hive.run()
    count = count + 1;	
	if count > 10 then
		hive.run = nil;
	end
    hive.sleep_ms(2000);	
end
```

## 错误处理

一旦`hive.run`函数执行时抛出错误,hive将立即退出,并将错误信息写入entry.lua.err文件中.  
这里entry.lua指的是入口文件的文件名,建议一般的服务程序在`hive.run`函数中自行捕获错误.


