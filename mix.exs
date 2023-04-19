defmodule ExYyjson.MixProject do
  use Mix.Project

  def project do
    [
      app: :ex_yyjson,
      version: "0.1.0",
      elixir: "~> 1.14",
      start_permanent: Mix.env() == :prod,
      compilers: [:elixir_make] ++ Mix.compilers,
      deps: deps(),
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    spec = [extra_applications: []]

    if Mix.env() != :bench do
      spec
    else
      Keyword.put_new(spec, :applications, [:logger])
    end
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:elixir_make, "~> 0.7", runtime: false},
      {:benchee_html, "~> 1.0", only: :bench, runtime: false},
      {:benchee, "~> 1.0", only: :bench, runtime: false},
      {:exjsx, "~> 4.0", only: [:bench, :profile], runtime: false},
      {:jason, "~> 1.2", only: [:dev, :test, :bench, :profile], runtime: false},
      {:jiffy, "~> 1.0", only: [:bench, :profile], runtime: false},
      {:json, "~> 1.4", only: [:bench, :profile], runtime: false},
      {:jsone, "~> 1.7", only: [:bench, :profile], runtime: false},
      {:poison, "~> 5.0", only: [:bench, :profile], runtime: false},
      {:tiny, "~> 1.0", only: [:bench, :profile], runtime: false}
    ]
  end
end
