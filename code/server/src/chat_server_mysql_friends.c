#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logger.h"
#include "chat_server_mysql_friends.h"

extern Connection_pool *mysql_pool;

/**************************
**   friends表的操作函数  **
***************************/
int insert_friend(mysql_friends* friend)
{
    MYSQL_BIND bind[6];
    MYSQL_STMT * stmt;

    stmt = get_stmt();
    
	const char *insert_query = "INSERT INTO friends (user1, user2, black1, black2, white1, white2) VALUES (?, ?, ?, ?, ?, ?);";
    if (mysql_stmt_prepare(stmt, insert_query, strlen(insert_query))) {
        LOG_ERROR("mysql_stmt_prepare() failed for insert_friend");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 绑定参数
    memset(bind, 0, sizeof(bind));

    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = friend->user_id_1;
    bind[0].buffer_length = strlen(friend->user_id_1);

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = friend->user_id_2;
    bind[1].buffer_length = strlen(friend->user_id_2);
    
    bind[2].buffer_type = MYSQL_TYPE_TINY;
    bind[2].buffer = &(friend->black_1);
    bind[2].buffer_length = sizeof(friend->black_1);

    bind[3].buffer_type = MYSQL_TYPE_TINY;
    bind[3].buffer = &(friend->black_2);
    bind[3].buffer_length = sizeof(friend->black_2);

    bind[4].buffer_type = MYSQL_TYPE_TINY;
    bind[4].buffer = &(friend->white_1);
    bind[4].buffer_length = sizeof(friend->white_1);

    bind[5].buffer_type = MYSQL_TYPE_TINY;
    bind[5].buffer = &(friend->white_2);
    bind[5].buffer_length = sizeof(friend->white_2);

    if (mysql_stmt_bind_param(stmt, bind)) {
        LOG_ERROR("mysql_stmt_bind_param() failed");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return EXIT_FAILURE;
    }

    // 插入第一条记录
    if (mysql_stmt_execute(stmt)) {
        LOG_ERROR("mysql_stmt_execute() failed for insert");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
    } else {
        LOG_DEBUG("Inserted friends success");
    }

    if(mysql_stmt_reset(stmt))
    {
        LOG_ERROR("mysql_stmt_reset() failed for insert");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
    }
    
    release_stmt(stmt);
    return EXIT_SUCCESS;
}

mysql_friends* find_friends_by_id(char* id)
{
    LOG_DEBUG("ENTER find_friends_by_id");
    MYSQL_BIND bind[2];
    my_bool is_null[6];
    my_bool error[6];
    unsigned long length[6];
    MYSQL_BIND result[6];
    mysql_friends friend;
    mysql_friends* friend_list_head = NULL;
    mysql_friends* friend_list_tail = NULL;
    mysql_friends* ptr = NULL;
    MYSQL_STMT * stmt;

    stmt = get_stmt();

    memset(bind, 0, sizeof(bind));

	const char* find_query = "select * from friends where user1 = ? or user2 = ?;";
	if (mysql_stmt_prepare(stmt, find_query, strlen(find_query))) {
        LOG_ERROR("mysql_stmt_prepare() failed for find_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    unsigned long user_id_len = strlen(id);
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = id;
    bind[0].buffer_length = sizeof(id);
    bind[0].length = &user_id_len;
    bind[0].is_null = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = id;
    bind[1].buffer_length = sizeof(id);
    bind[1].length = &user_id_len;
    bind[1].is_null = 0;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        LOG_ERROR("mysql_stmt_bind_param() failed in find_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }
    
    if (mysql_stmt_execute(stmt)) {
        LOG_ERROR("mysql_stmt_execute() failed for find_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    // 定义绑定结果集的结构体
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = friend.user_id_1;
    result[0].buffer_length = sizeof(friend.user_id_1);
    result[0].is_null = &is_null[0];
    result[0].length = &length[0];
    result[0].error = &error[0];

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = friend.user_id_2;
    result[1].buffer_length = sizeof(friend.user_id_2);
    result[1].is_null = &is_null[1];
    result[1].length = &length[1];
    result[1].error = &error[1];

    result[2].buffer_type = MYSQL_TYPE_TINY;
    result[2].buffer = &friend.black_1;
    result[2].buffer_length = sizeof(friend.black_1);
    result[2].is_null = &is_null[2];
    result[2].length = &length[2];
    result[2].error = &error[2];

    result[3].buffer_type = MYSQL_TYPE_TINY;
    result[3].buffer = &friend.black_2;
    result[3].buffer_length = sizeof(friend.black_2);
    result[3].is_null = &is_null[3];
    result[3].length = &length[3];
    result[3].error = &error[3];

    result[4].buffer_type = MYSQL_TYPE_TINY;
    result[4].buffer = &friend.white_1;
    result[4].buffer_length = sizeof(friend.white_1);
    result[4].is_null = &is_null[4];
    result[4].length = &length[4];
    result[4].error = &error[4];

    result[5].buffer_type = MYSQL_TYPE_TINY;
    result[5].buffer = &friend.white_2;
    result[5].buffer_length = sizeof(friend.white_2);
    result[5].is_null = &is_null[5];
    result[5].length = &length[5];
    result[5].error = &error[5];

    // 绑定结果集
    if (mysql_stmt_bind_result(stmt, result)) {
        LOG_ERROR("mysql_stmt_bind_result() failed");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    // 获取结果
    while (mysql_stmt_fetch(stmt) == 0)
    {
        ptr = (mysql_friends*)malloc(sizeof(mysql_friends));
        strcpy(ptr->user_id_1, friend.user_id_1);
        strcpy(ptr->user_id_2, friend.user_id_2);
        ptr->black_1 = friend.black_1;
        ptr->black_2 = friend.black_2;
        ptr->white_1 = friend.white_1;
        ptr->white_2 = friend.white_2;

        if(friend_list_head == NULL)
        {
            friend_list_head = ptr;
            friend_list_tail = friend_list_head;
        } else 
        {
            friend_list_tail->next = ptr;
            ptr->next = NULL;
            friend_list_tail = ptr;
        }
    }

    LOG_DEBUG("find_friends_by_id success");

    if(mysql_stmt_reset(stmt))
    {
        LOG_ERROR("mysql_stmt_reset() failed for find");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
    }
    
    LOG_DEBUG("leave find_friends_by_id");
    release_stmt(stmt);
    return friend_list_head;
}

char** find_common_friends_by_id(char* id)
{
    LOG_DEBUG("ENTER find_common_friends_by_id");
    MYSQL_BIND bind[3];
    my_bool is_null[6];
    my_bool error[5];
    unsigned long length[5];
    MYSQL_BIND result[1];
    char friend_id[10];
    MYSQL_STMT * stmt;

    stmt = get_stmt();

    memset(bind, 0, sizeof(bind));

	const char* find_query = "select \
                                case \
                                    when user1 != ? then user1 else user2 \
                                end as result \
                              from friends where (user1 = ? or user2 = ?) \
                              and (black1 = 0 and black2 = 0);";
	if (mysql_stmt_prepare(stmt, find_query, strlen(find_query))) {
        LOG_ERROR("mysql_stmt_prepare() failed for find_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    unsigned long user_id_len = strlen(id);
    
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = id;
    bind[0].buffer_length = sizeof(id);
    bind[0].length = &user_id_len;
    bind[0].is_null = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = id;
    bind[1].buffer_length = sizeof(id);
    bind[1].length = &user_id_len;
    bind[1].is_null = 0;

    bind[2].buffer_type = MYSQL_TYPE_STRING;
    bind[2].buffer = id;
    bind[2].buffer_length = sizeof(id);
    bind[2].length = &user_id_len;
    bind[2].is_null = 0;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        LOG_ERROR("mysql_stmt_bind_param() failed in find_common_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }
    
    if (mysql_stmt_execute(stmt)) {
        LOG_ERROR("mysql_stmt_execute() failed for find_common_friends_by_id");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    // 定义绑定结果集的结构体
    memset(result, 0, sizeof(result));

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = friend_id;
    result[0].buffer_length = sizeof(friend_id);
    result[0].is_null = &is_null[0];
    result[0].length = &length[0];
    result[0].error = &error[0];

    // 绑定结果集
    if (mysql_stmt_bind_result(stmt, result)) {
        LOG_ERROR("mysql_stmt_bind_result() failed");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
        release_stmt(stmt);
        return NULL;
    }

    // 会自动将元素初始化为NULL
    char** friend_id_list = (char**)calloc(20, sizeof(char**));
    int count = 0;

    // 获取结果
    while (mysql_stmt_fetch(stmt) == 0)
    {
        friend_id_list[count] = (char*)malloc(sizeof(10 * sizeof(char)));
        strcpy(friend_id_list[count], friend_id);
        count++;
    }

    LOG_DEBUG("find_common_friends_by_id success");

    if(mysql_stmt_reset(stmt))
    {
        LOG_ERROR("mysql_stmt_reset() failed for find");
        LOG_ERROR("Error: %s", mysql_stmt_error(stmt));
    }
    
    LOG_DEBUG("leave find_common_friends_by_id");
    release_stmt(stmt);
    return friend_id_list;
}