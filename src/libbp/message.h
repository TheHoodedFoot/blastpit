#ifndef MESSAGE_H
#define MESSAGE_H

/* extern pointer to our class template */
extern const void *Message;

typedef struct Message {   /* mqtt object struct */
	const void *klass; /* must be first */
	long size;
	char *data;
} t_Message;

int isMessageEqual(t_Message *, const char *);
char *getMessageData(t_Message *);
int getMessageSize(t_Message *);

#endif /* end of include guard: MESSAGE_H */
