#ifndef JME_Encrypt_h__
#define JME_Encrypt_h__

#include <string>
using namespace std;

namespace JMEngine
{
	namespace encrypt
	{
		struct SBlock 
		{ 
			SBlock(unsigned int l=0, unsigned int r=0) : m_uil(l), m_uir(r) {} 

			SBlock(const SBlock& roBlock) : m_uil(roBlock.m_uil), m_uir(roBlock.m_uir) {} 
			SBlock& operator^=(SBlock& b) { m_uil ^= b.m_uil; m_uir ^= b.m_uir; return *this; } 
			unsigned int m_uil, m_uir; 
		}; 

		class CBlowFish 
		{ 
		private: 
			enum { ECB=0, CBC=1, CFB=2 }; 
			CBlowFish() {}
			CBlowFish(unsigned char* ucKey, size_t keysize, const SBlock& roChain = SBlock(0UL,0UL));
			~CBlowFish() {} 

		public:
			static void Encrypt( const char* strInput, string& strOutput, const char* strKey, int bitCount, int len );
			static void Decrypt( const char* strInput, string& strOutput, const char* strKey, int bitCount, int len ); 

		private:
			void ResetChain() { m_oChain = m_oChain0; } 

			void Encrypt_2(unsigned char* buf, size_t n, int iMode=ECB); 
			void Decrypt_2(unsigned char* buf, size_t n, char x_char = 0, int iMode=ECB); 
			void Encrypt_3(const unsigned char* in, unsigned char* out, size_t n, int iMode=ECB); 
			void Decrypt_3(const unsigned char* in, unsigned char* out, size_t n, int iMode=ECB); 

		private: 
			unsigned int F(unsigned int ui); 
			void Encrypt_4(SBlock&); 
			void Decrypt_4(SBlock&); 

		private: 
			SBlock m_oChain0; 
			SBlock m_oChain; 
			unsigned int m_auiP[18]; 
			unsigned int m_auiS[4][256]; 
			static const unsigned int scm_auiInitP[18]; 
			static const unsigned int scm_auiInitS[4][256]; 
		};
	}
}
#endif // JME_Encrypt_h__
