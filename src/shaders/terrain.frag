#version 330 core

// Lighting data (optional)
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
in vec3 Normal;
in vec3 WorldCoords;

// 2D Texture data
uniform sampler2D sampleTexture;
in vec2 TexCoords2D;

out vec4 outColor;

void main() {
    vec4 lighting = vec4(1.0);

    // Implement the Phong lighting model, if lighting details have been passed
    if (lightColor != vec3(0.0)) {
        /* Configurable parameters */
        float ambientStr = 0.4;
        float diffStr = 1.0;
        float specularStrength = 0.1;   // terrain really shouldn't have much specular lighting
        float specularShine = 32;   // Higher -> reflection is more "dense" (not diffused)

        /* 1) Ambient lighting */
        vec3 ambient = ambientStr * lightColor;

        /* 2) Diffuse lighting */
        // Calculate direction between light source and fragment's position (world position)
        vec3 norm = normalize(Normal);
        vec3 lightDir = normalize(lightPos - WorldCoords);

        // Calculate the diffuse light "impact" by taking the dot product of lightDir & the surface normal
        float diffImpact = dot(norm, lightDir);
        diffImpact = max(diffImpact, 0.0);    // make sure the value is not negative
        vec3 diffuse = diffStr * diffImpact * lightColor;

        /* 3) Specular lighting */
        // Calculate the amount of reflection we'd see from our viewing position
        vec3 viewDir = normalize(viewPos - WorldCoords);
        vec3 reflectDir = reflect(-lightDir, norm); // negative bc needs to be from light source to fragment
        float specImpact = max(dot(viewDir, reflectDir), 0.0);
        specImpact = pow(specImpact, specularShine);
        vec3 specular = specularStrength * specImpact * lightColor;

        lighting = vec4(ambient + diffuse + specular, 1.0);
    }

    outColor = texture(sampleTexture, TexCoords2D) * lighting;
}