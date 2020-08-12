# ot14
The octree mesh generation code

### Conformal 3D Mesh Generation for Free Style Architectural Design
---------------
[Paper](https://www.researchgate.net/publication/228118810_Conformal_adaptive_hexahedral-dominant_mesh_generation_for_CFD_simulation_in_architectural_design_applications) | [Presentation](https://youtu.be/DhGdQhYkTlA) 
<iframe width="560" height="315" src="https://www.youtube.com/embed/DhGdQhYkTlA" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

![3d_mesh](/images/3d_mesh.png "3d_mesh")

Mesh generation is a critical and probably the most manually intensive step in CFD simulations in the architectural domain. One essential feature is the large span of dimensional scales that is encountered in design, particularly if the model aims to simulate indoor and outdoor conditions concurrently, e.g., site at the magnitude of kilometers while building elements at the magnitude of centimeters. In addressing the challenge this paper presents an approach to generate adaptive hexahedral-dominate meshes for CFD simulations in sustainable architectural design applications. Uniform all-hexahedral meshes and adaptive hexahedral-dominant meshes are both generated for natural ventilation simulation of a proposed retroﬁt building in Philadelphia. Simulation results show that adaptive hexahedral-dominate meshes generate very similar results of air change rate in the space due to natural ventilation, compared to all-hexahedral meshes yet with up to 90% reduction in number of elements in the domain, hence improve computation efﬁciency.

### Adaptive 2D Mesh Generation Architecture Designs
---------------
[Paper](https://www.sciencedirect.com/science/article/abs/pii/S0360132310001149)

![2d_mesh](/images/2d_mesh.png "2d_mesh")

A prototype meshing tool is developed to construct adaptive quadrilateral meshes from two-dimensional image data, e.g., architecture drawings. First the quadtree based isocontouring method is utilized to generate initial uniform quadrilateral meshes in the immediate region of the objects. Meshes are further decomposed into finer quads adaptively near the surface of the object without introducing any hanging nodes. Boundary layers are then generated using the pillowing technique and the thickness of the boundary layer is controlled to achieve the desired yþ values for different near wall turbulence models. Finally, meshes are extended to the ambient domain with desired sizes, where flow fields are assumed to be relatively steady. The developed tool has been employed to generate meshes for CFD simulations of scenarios commonly existing in the indoor and outdoor environment.
