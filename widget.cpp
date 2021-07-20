#include "widget.h"
#include "ui_widget.h"

QVector<QTcpSocket*>all_tcpsocket;

struct chat_info{
    int m_num;
    int y_num;
    QString n_content;
    QString n_time;
};
QVector<chat_info> all_chat_info;

struct relation{
    int rm_num;
    int ry_num;
};
QVector<relation> all_relation;

struct chat_socket{
    QTcpSocket *csocket;
    int num;
};
QVector<chat_socket> struct_socket;

struct group{
    int group_leader;
    QString group_member;
    QString group_content;
};
QVector<group> all_group;

QVector<group> specific_group;


struct single_per{
    int num;
    int weight;
};

struct weight_single_per{
    QVector<single_per> tem_single;
    int per_num;
};

struct ans_of_close{
    int num;
    double score;
};
QVector<ans_of_close> all_score;

//bool compare(single_per a,single_per b){
//    return a.num<b.num;
//}


Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    server = new QTcpServer(this);
    server->listen(QHostAddress::Any, 9999);
    connect(server,SIGNAL(newConnection()),this,SLOT(acceptConnection()));

}

Widget::~Widget()
{
    delete ui;
}


//void myserver::NewConnectionSlot(){//已完成，勿动
//    currentClient=tcpServer->nextPendingConnection();
//    tcpClient.append(currentClient);
//    clientIP->addItem(currentClient->peerAddress().toString());
//    connect(currentClient,SIGNAL(readyRead()),this,SLOT(ReadData()));
//    connect(currentClient,SIGNAL(disconnected()),this,SLOT(disconnectedSlot()));
//}



void Widget::acceptConnection()
{
    qDebug()<<"3"<<endl;
    //想发送数据就还要用socket

    QTcpSocket * sk = server->nextPendingConnection();
    qDebug()<<"4"<<endl;
    //把现在的socket 加到vector中
    all_socket.push_back(sk);
    qDebug()<<"5"<<endl;
    connect(sk,SIGNAL(readyRead()),this,SLOT(replyToClient()));


}

void Widget::replyToClient()
{
    QTcpSocket * serverSocket = (QTcpSocket * )sender();
    int mark = 0;//mark 来区分不同的情况 1为看资料，2为聊天
    //打包列表信息需要一个size
    int peerlist_size = 0;
    QString peerlist;

    //传入了数据了 切这个数据为msg
    qDebug()<<"6"<<endl;
    QString msg = QString(serverSocket->readAll());

    qDebug()<<"7"<<msg<<endl;
    ui->textEdit->setText(msg);

    //定义一些往数据库表格里存放数据的元素      发送消息的人的num+接收消息的人的num+发送消息的内容+发送时刻的时间
    QString t_num,t_password,t_username,t_style,t_goal_name,t_is_on;
    QString t_goal_num,t_content,t_time;

    //用不同的方法对传入的数据进行解析

    //对群聊界面传入的数据进行解析
    if(msg[msg.size()-1] == "#"){    //如果传进来字串的最后一位是"#" => 群聊界面
        //先删除句末尾的标志符号
        QString group_msg = msg.mid(0,msg.size()-1);
        qDebug()<<group_msg<<endl;
        //然后用正则表达式表示出来
        /*
         * 第一部分是群主的id，
         * 第二部分是除了群主外还有多少人，
         * 第三部分是除了群主外所有人的信息(id+"#")*个数   但是建表的时候二三合成一部分
         * 第四部分是内容
         */
        qDebug()<<"1      进入程序 之后开始检验两个向量的大小 "<< " specific=" <<specific_group.size()<<"  all_group="<<all_group.size()<< endl;
        QString t_group_leader = group_msg.section("<",0,0);   //得到群主的id
        QString t_group_info = group_msg.section("<",1,1);   //得到所有成员的信息
        QString t_group_content = group_msg.section("<",2,2);   //得到聊天内容
        group temgroup;
        temgroup.group_leader = t_group_leader.toInt();
        temgroup.group_member = t_group_info;
        temgroup.group_content = t_group_content;
        all_group.push_back(temgroup);
        qDebug()<<"2 "<<all_group.size()<<endl;
        if(specific_group.size() == 0){
            //输出增加群主的种类
            qDebug()<<"开始添加群主的种类"<<endl;
            specific_group.push_back(all_group[0]);
        }
        qDebug()<<"3"<<endl;
        //先选出特定的群主不同的
        for (int i = 1;i < all_group.size();i++) {
            for (int j = 0;j < specific_group.size();j++) {
                if(all_group[i].group_leader == specific_group[j].group_leader){
                    //已经有了，就不增加specific的量了
                }else {
                    specific_group.push_back(all_group[i]);
                }
            }
        }
        qDebug()<<"4"<<endl;
        //然后得到了有多少个群
        for (int i = 0;i <specific_group.size();i++) {
            //封装发送到各个客户端的字符串应该是先  各成员的信息，然后才是群主信息（这样方便遍历）
            QString group_chat_info = "";
            for (int j = 0;j < all_group.size();j++) {
                if(specific_group[i].group_leader == all_group[j].group_leader){
                    //如果是这个群的消息
                    group_chat_info += all_group[j].group_member;
//                    group_chat_info += QString::number(specific_group[i].group_leader);
                    group_chat_info += "*";   //用"*"用来区分每条信息中的联系人和消息
                    group_chat_info += all_group[j].group_content;
                    group_chat_info += "$";   //用"$"来区分所有不同的群聊消息
                }
            }
            char* size_ch;
            QByteArray size_ba = group_chat_info.toLatin1();
            size_ch = size_ba.data();
            for(int p = 0;p < all_socket.size();p++){
                all_socket[p]->write(size_ch);    //每个都发送
            }
        }
        qDebug()<<"5"<<endl;
//        // 1. 加载 SQLite 数据库驱动
//        QSqlDatabase group_db = QSqlDatabase::addDatabase("QSQLITE", "Connection_Name");

//        // 2. sqlitedata.db 是 SQLite 的数据库文件名
//        //    如果此文件不存在则自动创建，如果已经存在则使用已经存在的文件
//        group_db.setDatabaseName("/Users/aczy156/s_server/sqlitedata_chat20.db");

//        // 3. 打开数据库连接，成功返回 true，不成功则程序退出
//        if (!group_db.open()) {
//            qDebug() << "Connect to Sqlite error: " << group_db.lastError().text();
//            exit(128);
//        }

//        // 4. 在数据库中创建表
//        QSqlQuery createTableQuery_chat(group_db);

//        QString sql_group("CREATE TABLE user("
//                         "id integer primary key autoincrement, "
//                         "group_leader text not null,"
//                         "group_info text not null,"
//                         "group_content text not null)");
//        createTableQuery_chat.exec(sql_group);
////        qDebug()<<"创建表格正常"<<endl;

//        // 5. 向表中插入一条数据
//        QSqlQuery insertQuery_group(group_db);
////        qDebug()<<"插入数据正常"<<endl;
//        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES (t_num, t_password ,t_username,t_style,t_goal_name)");
//        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES ('1', '2','3','4','5')");
//        QString temstr_chat = "INSERT INTO user (group_leader,group_info, group_content) VALUES ('"+t_group_leader+"','"+t_group_info+"','"+t_group_content+"')";
//        insertQuery_group.exec(temstr_chat);

        //遍历提取群主
    }




    //添加对客户端界面传来的数据进行
    if(msg[msg.size()-1] == "^"){   //如果结尾是"^" =>则代表的是查询亲密度排行
        qDebug()<<"开始输出所有的关系数据"<<endl<<"**********************************************************"<<endl;


        //首先得到要得到关于谁的，即被计算对象的num
        int get_mark_num = msg.mid(0,msg.size()-1).toInt();

        /*
         * 首先要对所有的数据进行遍历，
         * 采集每个人对应的和其他人聊天的次数的数据 => 有向关系，因为有时存在a对b有好感，但b对a冷淡，所以ab之间的关系分为a对b、b对a(直接影响，所以每有一条消息权值加2。)
         * 采集所在一个群的数量，在一个群里的所有成员双向权值增加(考虑到在一个群里势必是因为某种原因，但是权值要小)
         */


        //首先获取所有的人数  然后对每个人开对应的向量来储存所有信息加权值
        QVector<int> per;
        //首先要放入两个用来与后面做比对
        per.push_back(all_chat_info[0].m_num);
        per.push_back(all_chat_info[0].y_num);
        for(int i = 0;i<all_chat_info.size();i++){   //然后从1开始遍历，因为0的已经被push进向量中
            int flag1 = 1;
            int flag2 = 1;
            for (int j = 0;j<per.size(); j++) {
                if(all_chat_info[i].m_num == per[j]){   //不等于所有的，
                    flag1 = 0;
                }
                if(all_chat_info[i].y_num == per[j]){
                    flag2 = 0;
                }
            }
            if(flag1){
                per.push_back(all_chat_info[i].m_num);
            }
            if(flag2){
                per.push_back(all_chat_info[i].y_num);
            }
        }
        //现在得到了所有的数，然后开始给每一数赋权值
        /*
         * //要开一个二重向量，动态储存有多少个人的数据，然后动态储存每个人对应其他人的权值
         *
         * 要根据刚才的到的所有人的向量per,然后开始按人次来遍历所有的消息列表
         * 最后达到来填充二维数组的元素
         *
         * 两个结构体
         * struct single_per{
         *     int num;
         *     int weight;
         * };
         *
         * struct weight_single_per{
         *     QVector<single_per> *tem_single;
         *     int per_num;
         * };
         *
         * QVector<weight_single_per> all_single_per;
         *
         */
        //per的第一个肯定和all_chat_info的第一个对应
        QVector<weight_single_per> all_single_per;
        for (int i = 0;i < per.size();i++) {
            weight_single_per now_weight_single_per;
            //首先要先往储存的那个里边保存一个信息，用于比较
            single_per fir_single_per;
            fir_single_per.num = all_chat_info[0].y_num;  //此时保存的信息一定是所有聊天情况的第一个的y_num，因为per列表就是按顺序push的
            fir_single_per.weight = 5;
            //再push进去第一个特例的情况
            now_weight_single_per.tem_single.push_back(fir_single_per);
            for (int j = 1;j < all_chat_info.size(); j++) {
                //如果是自己给别人发送，权值加5 自己指向那个人（有向表示）
                if(all_chat_info[j].m_num == per[i]){
                    //先遍历一遍，看看这个人储存的数据向量中有没有这个人的数据
                    int is_find = 0;
                    for(int k = 0; k < now_weight_single_per.tem_single.size();k++){   //这个最里边的循环是用来处理这个人的与其他人联系的向量中是否已经有相联系的
                        //因为此时的发送人和per的第i个是一样的，所有此时要比对的是接收人是否有在这个向量中
                        if(all_chat_info[j].y_num == now_weight_single_per.tem_single[k].num){
                            //此时发现有匹配成功的，则权值加5
                            now_weight_single_per.tem_single[k].weight += 5;
                            is_find = 1;
                        }
                    }
                    //遍历了一遍，发现没有找到
                    if(is_find == 0){
                        //就要给这个重新创建一个对象，
                        single_per next_single_per;
                        next_single_per.num = all_chat_info[j].y_num;
                        next_single_per.weight = 5;
                        //再push进去
                        now_weight_single_per.tem_single.push_back(next_single_per);
                    }
                }
                //如果是别人给自己发的，权值加3，别人指向自己
                if(all_chat_info[j].y_num == per[i]){
                    int is_find = 0;
                    //还是先遍历一遍
                    for (int k = 0;k <now_weight_single_per.tem_single.size();k++) {
                        //此时要判断的应该是发送人是否已经存在于第i个人的数据向量中了
                        if(all_chat_info[j].m_num == now_weight_single_per.tem_single[k].num){
                            //如果发现数据向量已经有了
                            now_weight_single_per.tem_single[k].weight += 3;
                            is_find = 1;
                        }
                    }
                    //再写没有找到的情况
                    if(is_find == 0){
                        //重建一个对象
                        single_per next_single_per;
                        next_single_per.num = all_chat_info[j].m_num;
                        next_single_per.weight = 3;
                        now_weight_single_per.tem_single.push_back(next_single_per);
                    }
                }
            }
            now_weight_single_per.per_num = per[i];
            all_single_per.push_back(now_weight_single_per);
        }

        //现在拿到了储存所有相互联系的二维向量
        //先输出检验一下
        for (int i = 0; i < all_single_per.size();i++) {
            qDebug()<<all_single_per[i].per_num<<endl;
            for (int j = 0;j < all_single_per[i].tem_single.size();j++) {
                qDebug()<<all_single_per[i].tem_single[j].num <<"  "<<all_single_per[i].tem_single[j].weight<<endl;
            }
        }


        //开始利用数据向量
        /*
         * 利用公式    皮尔逊系数计算公式  p=(Σxy-Σx*Σy/n)/Math.sqrt((Σx2-(Σx)2/n)(Σy2-(Σy)2/n));
         *            传进公式的x，y为分别为两个向量相对应的权值，是相对应的
         * 没有相互联系要先对数据进行预处理，把没有联系的补0为1，<1>防止分母出现0的情况 <2>类比卷积中的边缘填充，补齐为一使卷积核遍历所有像素点的几率相等
         */

        //数据预处理  首先要把不是零的补齐，然后再利用结构体排序
        for (int i = 0; i < all_single_per.size();i++) {
            qDebug()<<all_single_per[i].per_num<<endl;
            //做比对补齐时的标准应该是和per结构体进行比对
            //先遍历所有的人(per的列表)，看看在里边有没有
            for(int j = 0;j < per.size();j++){
                int is_find = 0;
                for (int k = 0;k < all_single_per[i].tem_single.size();k++) {
                    if(per[j] == all_single_per[i].tem_single[k].num){
                        is_find = 1;
                    }
                }
                if(is_find == 0){
                    //如果没有找到相同的
                    //new一个出来，然后把他push到向量中
                    single_per the_last_single_per;
                    the_last_single_per.num = per[j];
                    //对于没有附上值的进行补全，补上1
                    the_last_single_per.weight = 1;
                    all_single_per[i].tem_single.push_back(the_last_single_per);
                }
            }
            //跳出第二层循环后对得到的数据进行结构体排序
//            qSort(all_single_per.begin(),all_single_per.end(),compare);
        }

        //经过数据预处理之后的数据情况
        qDebug()<<"开始输出所有的经过数据预处理之后的数据"<<endl<<"*******************************************"<<endl;
        for (int i = 0; i < all_single_per.size();i++) {
            qDebug()<<all_single_per[i].per_num<<endl;
            for (int j = 0;j < all_single_per[i].tem_single.size();j++) {
                qDebug()<<all_single_per[i].tem_single[j].num <<"  "<<all_single_per[i].tem_single[j].weight<<endl;
            }
        }
        //到此处向量正常




        //最后带入公式开始计算
        //要求get_mark_num与除他之外的所有人的关系
        //首先，计算出每个人对应此时的和，平方和
        int main_1 = 0,main_2 = 0,mark_position = 0;
        qDebug()<<"1"<<endl;
        for(int i = 0;i < all_single_per.size();i++){  //首先要找到被检查的对象，然后计算与他相关的值
            if(all_single_per[i].per_num == get_mark_num){
                mark_position = i;
                for (int j = 0;j < all_single_per[i].tem_single.size();j++) {
                    main_1 += all_single_per[i].tem_single[j].weight;
                    main_2 += pow(all_single_per[i].tem_single[j].weight,2);
                }
            }
        }
        qDebug()<<"2"<<" 在2的时候输出一下此时的mark_position"<< mark_position <<endl;
        //此时得到了被检查对象的几个参数值的和和平方和
        for(int i = 0;i<all_single_per.size();i++){
            if(all_single_per[i].per_num != get_mark_num){  //不和自己比较
                int vice_1 = 0,vice_2 = 0,multi_total = 0;
                for(int j = 0;j < all_single_per[i].tem_single.size();j++){     //先计算出这个情况下的几个参数值的和和平方和
                    vice_1 += all_single_per[i].tem_single[j].weight;
                    vice_2 += pow(all_single_per[i].tem_single[j].weight,2);
                }
                qDebug()<<i<<"  3"<<endl;                   //输出完0 3之后就越界报错
                //此时得到了遍历到的对象的各参数的和和平方和
                //然后再计算对应的人的乘积   要先遍历
                for (int j = 0;j <all_single_per[i].tem_single.size();j++) {
                    for (int k = 0;k < all_single_per[mark_position].tem_single.size();k++) {
                        //在标准中遍历
                        if(all_single_per[i].tem_single[j].num == all_single_per[mark_position].tem_single[k].num){
                            //遍历到了之后把权值加起来
                            multi_total += all_single_per[i].tem_single[j].weight*all_single_per[mark_position].tem_single[k].weight;
                        }
                    }
                }
                qDebug()<<i<<"  4"<<endl;
                //此处拿到了五个数，分别是vice_1,vice_2,main_1,main_2,multi_total;
                qDebug()<<vice_1<<"  "<<vice_2<<"  "<<main_1<<"  "<<main_2<<"  "<<multi_total<<endl;
                //然后带入公式，计算此时的i与get_mark_num的皮尔逊系数值
                double sd = multi_total - main_1 * vice_1 / all_single_per.size();  //先计算分子
                //再计算分母
                double sm = sqrt((main_2 - pow(main_1, 2) / all_single_per.size()) * vice_2 - pow(vice_1, 2) / all_single_per.size());
                //得到的是小数，要用double类型
                double tem = abs(sm == 0 ? 1 : sd / sm);
                ans_of_close tem_ans;
                tem_ans.num = all_single_per[i].per_num;
                tem_ans.score = tem;
                all_score.push_back(tem_ans);
            }
        }
        qDebug()<<"5"<<endl;
        //得到了相应的得分之后存放在了all_score 向量中
        //输入结果向量
        QString ans_close = QString::number(get_mark_num)+"#";
        for (int i = 0;i < all_score.size();i++) {
            qDebug()<<all_score[i].num <<" 的分数为 "<<all_score[i].score<<endl;
            ans_close += QString::number(all_score[i].num);
            ans_close += "*";
            ans_close += QString::number(all_score[i].score);
            ans_close +="#";
        }
        //整合得到了字符串，然后拿到客户端解析
        QByteArray ba = ans_close.toLatin1();
        char * close_char = ba.data();
        for (int i = 0; i < all_socket.size();i++) {
            all_socket[i]->write(close_char);
        }
    }




    //对聊天界面传来的数据进行解析
    //此处可以得到客户端传来的数据，所以在这里可以对传来的数据打包放进数据库中
    /*
         *
         * 此部分是创建一个储存所有聊天信息的数据库
         * 注意元素组成：发送消息的人的num+接收消息的人的num+发送消息的内容+发送时刻的时间
         * 这个数据库看两者的联系是通过解析表格的前两个然后来看这两个人的聊天
         * 这个思路也可以延伸到群聊系统，群聊系统
         *
         */
    if(msg[msg.size()-1] == "-"){ //如果传进来字串的最后一位是"-" => 聊天界面
        qDebug()<<msg<<endl;
        //        //上来之后先把所有的消息请求清空一下    // 因为后面更新了算法，每次只push新添加进来的聊天请求，就不会再出问题了
        //        all_chat_info.clear();
        QString chat_msg = msg.mid(0,msg.size()-1);
        //为了防止出现聊天内容中间出现分隔符，就将分隔符定义为"$";
        t_num = chat_msg.section('$',0,0);
        t_goal_num = chat_msg.section('$',1,1);
        t_content = chat_msg.section('$',2,2);
        t_time = chat_msg.section('$',3,3);
        mark = 2;
        // 1. 加载 SQLite 数据库驱动
        // QSqlDatabase chat_db = QSqlDatabase::addDatabase("QSQLITE", "Connection_Name");

        QSqlDatabase chat_db;
        if(QSqlDatabase::contains("Connection_Name"))
          chat_db = QSqlDatabase::database("Connection_Name");
        else
          chat_db = QSqlDatabase::addDatabase("QSQLITE");

        // 2. sqlitedata.db 是 SQLite 的数据库文件名
        //    如果此文件不存在则自动创建，如果已经存在则使用已经存在的文件
        chat_db.setDatabaseName("sqlitedata_chat49.db");

        // 3. 打开数据库连接，成功返回 true，不成功则程序退出
        if (!chat_db.open()) {
            qDebug() << "Connect to Sqlite error: " << chat_db.lastError().text();
            exit(128);
        }

        // 4. 在数据库中创建表
        QSqlQuery createTableQuery_chat(chat_db);

        QString sql_chat("CREATE TABLE user("
                         "id integer primary key autoincrement, "
                         "num text not null,"
                         "goal_num text not null, "
                         "content text not null, "
                         "time text not null)");
        createTableQuery_chat.exec(sql_chat);
//        qDebug()<<"创建表格正常"<<endl;

        // 5. 向表中插入一条数据
        QSqlQuery insertQuery_chat(chat_db);
//        qDebug()<<"插入数据正常"<<endl;
        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES (t_num, t_password ,t_username,t_style,t_goal_name)");
        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES ('1', '2','3','4','5')");
        QString temstr_chat = "INSERT INTO user (num, goal_num,content,time) VALUES ('"+t_num+"','"+t_goal_num+"','"+t_content+"','"+t_time+"')";
        insertQuery_chat.exec(temstr_chat);
        chat_info tem_chat_info;
        tem_chat_info.m_num = t_num.toInt();
        tem_chat_info.y_num = t_goal_num.toInt();
        tem_chat_info.n_content = t_content;
        tem_chat_info.n_time = t_time;
        all_chat_info.push_back(tem_chat_info);

        //        insertQuery.exec("INSERT INTO user (username, password) VALUES ('Alice', 'passw0rd')");
        //    insertQuery.exec("INSERT INTO user (username, password,email) VALUES ('ken', 'passw1rd','2')");
        //    insertQuery.exec("INSERT INTO user (username, email,password) VALUES ('Alice','1', 'passw0rd')");
        //    insertQuery.exec("INSERT INTO user (username, email,password) VALUES ('ken','2', 'passw1rd')");

        //        '%% value+%%'

                // 6. 查询刚才插入的数据    //次数可以根据关键词进行遍历
                QSqlQuery selectQuery_chat(chat_db);
                selectQuery_chat.exec("SELECT id, num,goal_num,content,time FROM user");
        //        qDebug() << "查询数据正常"<<endl;
        //        while (selectQuery_chat.next()) {

        //            //把所有聊天的信息储存在vector
        //            chat_info tem_chat_info;
        //            tem_chat_info.m_num = selectQuery_chat.value("num").toInt();
        //            tem_chat_info.y_num = selectQuery_chat.value("goal_num").toInt();
        //            tem_chat_info.n_content = selectQuery_chat.value("content").toString();
        //            tem_chat_info.n_time = selectQuery_chat.value("time").toString();
        //            all_chat_info.push_back(tem_chat_info);

        //            qDebug() << QString("Id: %1, num: %2, goal_num: %3, content: %4, time: %5")
        //                        .arg(selectQuery_chat.value("id").toInt())
        //                        .arg(selectQuery_chat.value("num").toInt())
        //                        .arg(selectQuery_chat.value("goal_num").toInt())
        //                        .arg(selectQuery_chat.value("content").toString())
        //                        .arg(selectQuery_chat.value("time").toString());
        //        }



        //现在拿到了所有的关系
        if(all_relation.size() == 0){
            relation firrelation;
            firrelation.rm_num = all_chat_info[0].m_num;
            firrelation.ry_num = all_chat_info[0].y_num;
            all_relation.push_back(firrelation);
        }
        qDebug()<<"8"<<endl;
        //首先先遍历一遍所有的情况，找到所有的又相互想聊天关系的
        for (int i = 1;i < all_chat_info.size();i++) {
            //第一个永远都进不去 => 所以要先把第一个push进去  然后从第二个开始遍历
            for (int j = 0;j < all_relation.size();j++) {
                //前面是请求被请求情况相同的 或者 请求被请求的情况相反  但都是相同的两个人，id一样的话，就不push
                if((all_chat_info[i].m_num == all_relation[j].rm_num&&all_chat_info[i].y_num == all_relation[j].ry_num)||(all_chat_info[i].m_num == all_relation[j].ry_num&&all_chat_info[i].y_num == all_relation[j].rm_num)){
                    //相同的话就不操作
                }else {
                    relation temrelation;
                    temrelation.rm_num = all_chat_info[i].m_num;
                    temrelation.ry_num = all_chat_info[i].y_num;
                    all_relation.push_back(temrelation);
                }
            }
        }
//        qDebug()<<"中间输出一下此时得到的关系向量的大小 （有多少对关系）  和   当前的所有聊天的请求数"<<all_relation.size()<<"  "<<all_chat_info.size()<<endl;
        int is_send = 1;
        //现在拿到了所有的关系的vector
        qDebug()<<"9"<<endl;
        for(int i = 0;i < all_relation.size();i++){
            QString tem_relation_str;
            for (int j = 0;j < all_chat_info.size();j++) {
                if((all_chat_info[j].m_num == all_relation[i].rm_num&&all_chat_info[j].y_num == all_relation[i].ry_num)||(all_chat_info[j].m_num == all_relation[i].ry_num&&all_chat_info[j].y_num == all_relation[i].rm_num)){
                    //将所有的相同的联系累加到一块  组成一个整体字符串，然后再传到相对应的聊天界面
                    QString one_relation_str = QString::number(all_chat_info[j].m_num)+"$"+QString::number(all_chat_info[j].y_num)+"$"+all_chat_info[j].n_content+"$"+all_chat_info[j].n_time;
                    tem_relation_str += one_relation_str;
                    tem_relation_str += "^";
//                    qDebug()<<"中间输出一下 此时遍历到了哪里 以及此时的总的str"<<i<<"  "<<j<<"  "<<tem_relation_str<<endl;
                }
            }
            //然后在此处对处理好的QString进行发送 发送者为all_relation中的两个元素
            QString packaged_peerlist = tem_relation_str;
//            qDebug() <<"打包好的所有聊天记录在这里……"<< packaged_peerlist<<endl;
            char* size_ch;
            QByteArray size_ba = packaged_peerlist.toLatin1();
            size_ch=size_ba.data();
    //        serverSocket->write(size_ch);
            //在此处对所有的socket都进行发送    然后再在客户端判断是否是自己想接收到的信息
            for (int p = 0;p < all_socket.size();p++) {
                all_socket[p]->write(size_ch);
            }
        }
        qDebug()<<"10"<<endl;

    }




    //对登陆界面传来的数据进行解析
    //对得到的msg进行解析，解析得到用户的idnum，密码，name，个签，自己的ID，自己的port，(目标IP，目标port)先换成goal_name
    else if(msg[msg.size()-1] == "+"){  //如果传进来字串的最后以为是"+" =>登陆界面
        QString login_msg = msg.mid(0,msg.size()-1);
        t_num = login_msg.section('*',0,0);
        t_password = login_msg.section('*',1,1);
        t_username = login_msg.section('*',2,2);
        t_style = login_msg.section('*',3,3);
        t_goal_name = login_msg.section('*',4,4);
        t_is_on = login_msg.section('*',5,5);
        qDebug()<<t_num<<"  "<<t_password<<"  "<<t_username<<"  "<<t_style<<"  "<<t_goal_name<<"  "<<t_is_on<<endl;
        mark = 1;
        // 1. 加载 SQLite 数据库驱动
//        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "Connection_Name1");

        QSqlDatabase db;
        if(QSqlDatabase::contains("Connection_Name1"))
          db = QSqlDatabase::database("Connection_Name1");
        else
          db = QSqlDatabase::addDatabase("QSQLITE");

        // 2. sqlitedata.db 是 SQLite 的数据库文件名
        //    如果此文件不存在则自动创建，如果已经存在则使用已经存在的文件
        db.setDatabaseName("sqlitedata_info99.db");

        // 3. 打开数据库连接，成功返回 true，不成功则程序退出
        if (!db.open()) {
            qDebug() << "Connect to Sqlite error: " << db.lastError().text();
            exit(128);
        }

        // 4. 在数据库中创建表
        QSqlQuery createTableQuery(db);

        QString sql("CREATE TABLE user("
                    "id integer primary key autoincrement, "
                    "num text not null,"
                    "password text not null, "
                    "username text not null, "
                    "style text not null, "
                    "goal_name text not null,"
                    "is_on text not null)");
        createTableQuery.exec(sql);
        qDebug()<<"创建表格正常"<<endl;



        //在插入数据之前先遍历一遍，看看是否已经有了注册的信息
        //传进来的是t_num  然后与数据库中的进行比对
        QSqlQuery selectQuery_check(db);
        selectQuery_check.exec("SELECT num FROM user");
        qDebug() << "查询数据正常"<<endl;
        while (selectQuery_check.next()) {
            if(selectQuery_check.value("num").toInt() == t_num.toInt()){
                //检查了一遍发现已经有了
                //首先先往回write信息
//                for (int i = 0;i<all_socket.size();i++) {
//                    all_socket[i]->write("111$");
//                }
                serverSocket->write("111%");
                //然后再退出
                return;
            }
        }










        // 5. 向表中插入一条数据
        QSqlQuery insertQuery(db);
        qDebug()<<"插入数据正常"<<endl;
        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES (t_num, t_password ,t_username,t_style,t_goal_name)");
        //    insertQuery.exec("INSERT INTO user (num, password,username,style,goal_name) VALUES ('1', '2','3','4','5')");
        QString temstr = "INSERT INTO user (num, password,username,style,goal_name,is_on) VALUES ('"+t_num+"','"+t_password+"','"+t_username+"','"+t_style+"','"+t_goal_name+"','"+t_is_on+"')";
        insertQuery.exec(temstr);
        //        insertQuery.exec("INSERT INTO user (username, password) VALUES ('Alice', 'passw0rd')");
        //    insertQuery.exec("INSERT INTO user (username, password,email) VALUES ('ken', 'passw1rd','2')");
        //    insertQuery.exec("INSERT INTO user (username, email,password) VALUES ('Alice','1', 'passw0rd')");
        //    insertQuery.exec("INSERT INTO user (username, email,password) VALUES ('ken','2', 'passw1rd')");

        //        '%% value+%%'

        // 6. 查询刚才插入的数据    //次数可以根据关键词进行遍历
        QSqlQuery selectQuery(db);
        selectQuery.exec("SELECT id, num,password,username ,style,goal_name,is_on FROM user");
        qDebug() << "查询数据正常"<<endl;
        while (selectQuery.next()) {
            peerlist_size++;
            //        qDebug() << QString("Id: %1, Username: %2, Password: %3,Email: %4")
            qDebug() << QString("Id: %1, num: %2, password: %3, username: %4, style: %5, goal_name: %6,is_on: %7")
                        .arg(selectQuery.value("id").toInt())
                        .arg(selectQuery.value("num").toInt())
                        .arg(selectQuery.value("password").toString())
                        .arg(selectQuery.value("username").toString())
                        .arg(selectQuery.value("style").toString())
                        .arg(selectQuery.value("goal_name").toString())
                        .arg(selectQuery.value("is_on").toString());

            //把每一个人的信息都暂时保存在一个临时tem_peer里
            QString tem_peer = selectQuery.value("num").toString()+"*"+selectQuery.value("username").toString()+"*"+selectQuery.value("style").toString()+"*"+selectQuery.value("goal_name").toString()+"*"+selectQuery.value("is_on").toString();
            qDebug()<<tem_peer;
            peerlist += "-";
            peerlist += tem_peer;
        }
        // 首先把数据列表打包发回给每个客户端=>发列表需要发送现在的人数，和其他人的所有资料。(方便做后续的资料系统)
        QString size_Str = QString::number(peerlist_size,10);
        QString packaged_peerlist = size_Str+peerlist+"%";   //%是为了区分发送到客户端那个socket 是谁想要的
        qDebug() << packaged_peerlist<<endl;
        char* size_ch;
        QByteArray size_ba = packaged_peerlist.toLatin1();
        size_ch=size_ba.data();
        for (int i = 0;i<all_socket.size();i++) {
            all_socket[i]->write(size_ch);
            //看发送了几次
            qDebug()<<"发送了一次"<<endl;
        }
    }
}
