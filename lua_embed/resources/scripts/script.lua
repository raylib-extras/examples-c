-- simple lua scripts

-- call a draw function int he application and use a value provided by the application from a global variable
DrawText('Hello, World Width='..ScreenWidth, 10, 10)

-- draw text using functions from the API, including one that returns a value
DrawText('raylib dt = '.. GetDeltaTime(), 10, 30)

-- draw a circle using a function in the application and some logic in lua

radius = math.sin(os.clock()) * 25

DrawCircle(ScreenWidth/2,400, radius + 100)

-- set some data into a global value using a lua function that the application will read
result = 'I came from lua and the clock is '..os.clock()
