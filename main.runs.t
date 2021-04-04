function Test(X: String, Z: Number) {
	print("Starting!")
	print(X)
	function J {
		print(X + " from J")
		function K(Y: Number) {
			print(Y)
		}
		K(Z)
	}
	J()
}

Test("Hello World", 15)
print("End")