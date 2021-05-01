#ifndef BLOCK_H_INCLUDED
#define BLOCK_H_INCLUDED

#define OBJECT_BLOCK_SIZE 2

#define BLOCK_REMOVE_AT( block, index, name ) block->name[index] = block->name[block->iTail];
#define BLOCK_MEMBER( t, n, block_size ) t n[block_size]
#define BLOCK_MEMBER_ARRAY( t, n, s, block_size ) t n[block_size][s]

#define MEMBER( t, n ) t n[OBJECT_BLOCK_SIZE]
#define MEMBER_ARRAY( t, n, s ) t n[OBJECT_BLOCK_SIZE][s]
#define REMOVE( block, index, name ) block->name[index] = block->name[block->iTail];

#define BLOCK_BODY() int iTail;

#endif
