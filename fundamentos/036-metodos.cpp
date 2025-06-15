#include <iostream>

class Persona{
	private:
    	int edad;  // Propiedad pública
   public:
		void setEdad(int nuevaedad){
			edad = nuevaedad;
		}
		int getEdad(){
			return edad;
		}
};

int main() {
	Persona Joshuedaniel;
	Joshuedaniel.setEdad(24);
	std::cout << "La edad es de " << Joshuedaniel.getEdad() << " años" << std::endl;
    return 0;
}
