#pragma once
#include <cassert>
#include <cstdio>


template<size_t DATA_SIZE = 64 , size_t POOL_SIZE = 256>
class MemoryPool {
public :
	enum {
		datasize = DATA_SIZE , 
		poolsize = POOL_SIZE , 
	};
	struct memoryblockS {
		memoryblockS* prev;
		memoryblockS* next;
		unsigned char content[datasize];
	};
	struct memory_collection{
		memoryblockS pool[poolsize];

		memoryblockS* get( int index ){
			if( index < 0 || index >= poolsize ) return NULL;
			return &pool[index];
		}
		memory_collection(){
			assert( poolsize > 0 );
			assert( datasize > 0 );
			pool[0].prev = NULL;
			for( int i = 0 ; i < poolsize ; i++ ){
				memoryblockS* current_container = get( i );
				memoryblockS* child = get( i + 1 );
				current_container->next = child;
				if( child ){
					child->prev = current_container;
				}
			}
		}
		void addres_dump(){
			for( int i = 0 ; i < poolsize ; i++ ){
				memoryblockS* current_container = get( i );
			}
		}
	};
private :
	memory_collection m_collection;
	memoryblockS* m_current;
public :
	MemoryPool(){
		m_current = m_collection.get(0);
	}
	void* Malloc(){
		return _getItem();
	}
	void Free( void* p ){
		_insertItem( reinterpret_cast<memoryblockS*>( p ) );
	}
private :
	memoryblockS* _getItem(){
		if( !m_current ){
			return NULL;
		}
		memoryblockS* result = m_current;
		memoryblockS* prev = m_current->prev;
		memoryblockS* next = m_current->next;
		m_current = next;
		if( m_current ){
			m_current->prev = prev;
		}
		return result;
	}
	void _insertItem( memoryblockS* item ){
		assert( item );
		memoryblockS* next = NULL;
		if( m_current ){
			m_current->next = item;
		}
		item->prev = m_current;
		item->next = next;
		m_current = item;
	}
};
