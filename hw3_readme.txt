Render times!
-- Timed with QElapsedTime
-- Executed with TBB
-- Disclaimer: My hardware is old.

Many Spheres

Uniform
   	    BVH		NO_BVH
1	    3.34938	4.14852      
4	    13.3324	18.1865
16	    58.0505	70.6737

Random
	    BVH		NO_BVH
1	    3.35426	4.29253
4	    13.5260	18.3226
16	    55.0602	68.6598

Stratified
	    BVH		NO_BVH	
1	    3.3621	4.42883
4	    14.1561	17.9391
16	    58.3472	72.8985

Wahoo

Uniform
   	    BVH		NO_BVH
1	    3.51076	638.791 
4	    13.9316
16	    62.8496

Random
	    BVH		NO_BVH
1	    3.52991	637.312
4	    14.1389
16	    63.3254

Stratified
	    BVH		NO_BVH	
1	    3.54346	652.966
4	    14.0426
16	    65.8963

Extra credit: Implemented best-candidate sampling for image-wide stratified sampling. Uses the pbr method of creating a template pixel tile with n^2 number of samples chosen with the best candidate method, then applying this sample template to every pixel in the scene.
