//Ideal to dispatch multiple groups
//Rather than one huge group
//Many GPU’s can handle group sizes of 64
//Very small group sizes isn’t great
//One thread group runs per hardware shader unit
//Small groups = poor hardware saturation

//Structured buffers on the other hand utilize the unified cache architecture, which means the first read is slow, 
//but all subsequent reads are very fast (if the requested data is already in the cache).

//efines the dimensions of the thread groups 
//Can have N number of threads per dimension

[numthreads(8, 8, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}