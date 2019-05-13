create database chat CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
use chat;

create table user (
	id			int,
	account 	char(32) NOT NULL,
	password 	char(32) NOT NULL,
	name 		char(32) NOT NULL,
	gender 		tinyint(1) NOT NULL,
	birth 		date NOT NULL,
	PRIMARY KEY (id)
);

create table friends (
	user_id_1 	int NOT NULL,
	user_id_2 	int NOT NULL,
	CONSTRAINT FK_friends_user FOREIGN KEY (user_id_1) REFERENCES user(id),
	FOREIGN KEY (user_id_2) REFERENCES user(id),
	CONSTRAINT U_F UNIQUE (user_id_1,user_id_2)
);

create table message (
	id 	int NOT  NULL,
	sender_id 	int NOT NULL,
	receiver_id 	int NOT NULL,
	content 		varchar(255) NOT NULL,
	PRIMARY KEY (id),
	CONSTRAINT FK_message_user_sender FOREIGN KEY (sender_id) REFERENCES user(id),
	CONSTRAINT FK_message_user_receiver FOREIGN KEY (receiver_id) REFERENCES user(id)
);