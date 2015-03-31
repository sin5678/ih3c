Authors:
  * Yang Hu: coolcute at gmail.com
  * Xiao Yang: yangxiaogreetings at gmail.com

### Current Version : 0.1.4 ###
  1. Works normal under the OS with multiply NIC.
  1. Updated the WinPcap to 4.02

## Attention ##
  1. You must uninstall all other kinds of H3C client like inode、xlink and huaweiH3C, in order to make everything normal.
  1. If anyone want to join our group, please send us an email with your Google account name. All welcome.

Tested operating system:
Win XP, Win Vista, Win 2003, Win 2008, Win 7

Under the GPL V3 License.

---


作者:
  * 胡杨:coolcute at gmail.com
  * 杨潇:yangxiaogreetings at gmail.com

### 当前版本：0.1.4 ###
  1. 多网卡环境下已经可以正常工作。
  1. WinPcap版本更新为4.02。

## 注意 ##
  1. 经过测试，必须在系统卸载已有的H3C客户端才可以正常运行，包括inode、xlink、huaweiH3C。
  1. 如果你有改进iH3C的意向，请给我们写信，我们会在确认之后将你加进iH3C的项目里面来。欢迎感兴趣的朋友加入我们。

测试过的操作系统:
Win XP, Win Vista, Win 2003, Win 2008

本程序遵循GLPV3开放源代码，欢迎大家加入一起写代码。

h3c\_svr.exe是服务程序，请不要直接运行。(以下步骤必须在配置好了pw.data之后才能见效)
  1. 如果是通过安装包安装好的版本，请直接在cmd中运行: net start myh3c。
  1. 如果重新安装了系统，请运行 h3c\_svr.exe -i 注册成服务,然后如1般执行。
  1. 卸载服务请执行h3c\_svr.exe -u。
  1. 查看网卡列表请执行h3c\_svr.exe -l。

pw.data配置指南:
第一行:用户名.
第二行:密码.
第三行:使用H3C认证的网卡序号(如果0不行可以尝试1,2,3....).

例子:
  * user
  * pass
  * 0

H3CWatcher是监视网络状况并且自动重连的程序，就算不运行也不影响H3C认证。当然，有它在掉线时自动重新连接认证会更为方便。

